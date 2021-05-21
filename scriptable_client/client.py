import multiprocessing
from os import cpu_count
import random
import subprocess
import time
from random import choice

import pandas

import argparse


def determine_client_hello_random(log: str) -> str:
    for line in log.splitlines():
        if 'ClientHello.random=' in line:
            return line.split('=')[-1].replace(' ', ':').rstrip(':')
    raise AssertionError('There was no Client Hello randomness in the log of a client, aborting')


def run_single_session(request_index: int, sut_name: str, use_sentinel: bool, wait_time: float, enable_skip_ccs_fin: bool, enable_noskip_ccs_fin: bool) -> (str, str, bool):
    config_files = ['./config/base.conf',
                    f'./config/{sut_name}.conf']
    if enable_skip_ccs_fin:
        if enable_noskip_ccs_fin:
            # Coin flip
            skip_ccs_fin = bool(random.randint(0, 1))
        else:
            skip_ccs_fin = True
    else:
        if enable_noskip_ccs_fin:
            skip_ccs_fin = False
        else:
            print('At least one of --skip or --noskip must be selected')
            exit(1)
    test_cases = ['correct_padding',
                  'missing_separator',
                  'wrong_version_number',
                  'incorrect_padding_start',
                  'wrong_separator_position']
    current_case = choice(test_cases)
    config_files.append(f'./config/{current_case}.conf')
    if skip_ccs_fin:
        config_files.append('./config/skip_change_cipher_spec_and_finished.conf')
    if use_sentinel:
        call_array = ['./TlsTestToolSentinel']
    else:
        call_array = ['./TlsTestTool']
    print(f'Starting client {request_index} with test case {current_case}{", skipping CCS&FIN" if skip_ccs_fin else ""}')
    call_array.extend([f'--configFile={config_file}' for config_file in config_files])

    # Start the tls test tool using popen, save log output to string
    try:
        pipes = subprocess.Popen(call_array, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        std_out, std_err = pipes.communicate()
        std_out = std_out.decode('utf-8')
        std_err = std_err.decode('utf-8')
        if pipes.returncode != 0:
            print(std_out)
            print(f'Return code: {pipes.returncode}')
        if len(std_err):
            print(f'Stderr: {std_err}')
    except OSError as error:
        print(f'OSError {error}')
        std_out = b''

    client_hello_random = determine_client_hello_random(std_out)
    time.sleep(wait_time * 0.001)
    return client_hello_random, current_case, skip_ccs_fin


def run_multiple_clients(repetitions: int, sut_name: str, use_sentinel: bool, wait_time: float, skip: bool, noskip: bool, parallelization_factor: int):
    request_arguments = [[index, sut_name, use_sentinel, wait_time, skip, noskip] for index in range(repetitions)]
    with multiprocessing.Pool(processes=parallelization_factor) as pool:
        results = pool.starmap(run_single_session, request_arguments)
    results_dataframe = pandas.DataFrame(results, columns=['client_hello_random', 'label', 'skipped_ccs_fin'])
    return results_dataframe


parser = argparse.ArgumentParser()
parser.add_argument('-f', '--folder', required=True,
                    help='Folder that contains the input files Packets.pcap and Client Requests.csv '
                    'and that the output files will be written to')
parser.add_argument('-r', '--repetitions', type=int, required=True,
                    help='Number of handshakes to execute')
parser.add_argument('-s', '--sentinel', action='store_true',
                    help='Use the sentinel-protected, unrestricted version of the TLS test tool')
parser.add_argument('-n', '--name', required=True,
                    help='Name of the system under test, used to load the matching IP&port configuration')
parser.add_argument('-w', '--wait', type=int, default=0,
                    help='Wait time in milliseconds after each request before starting the next request')
parser.add_argument('--skip', action='store_true', default=False,
                    help='Make some request where the client omits ChangeCipherSpec and Finished')
parser.add_argument('--noskip', action='store_true', default=False,
                    help='Make some request where the client properly sends ChangeCipherSpec and Finished')
parser.add_argument('--processes', type=int, default=cpu_count(),
                    help='Parallelization factor, how many processes to use concurrently')
args = parser.parse_args()
request_results = run_multiple_clients(args.repetitions, args.name, args.sentinel, args.wait, args.skip, args.noskip, args.processes)
request_results.to_csv(f'{args.folder}/Client Requests.csv')
request_results.to_excel(f'{args.folder}/Client Requests.xlsx')

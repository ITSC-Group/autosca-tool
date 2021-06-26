from typing import List, Optional, Dict, Union

from network_trace import IterableCapture
from state_machine import StateMachine
from state_machine import State

from pyshark.packet.packet import Packet
from pyshark.packet.layer import Layer
from pyshark.packet.fields import LayerFieldsContainer

import pandas

from num2words import num2words

import argparse


class FeatureExtractor:
    def __init__(self, capture_file: str, label_file: str):
        self.iterable_capture = IterableCapture(capture_file)
        self.label_dataframe = pandas.read_csv(label_file)

    @staticmethod
    def is_from_server(packet: Packet, server_ip: str) -> bool:
        if 'ip' in packet:
            return packet.ip.src.get_default_value() == server_ip
        if 'ipv6' in packet:
            return packet.ipv6.src.get_default_value() == server_ip

    @staticmethod
    def is_from_client(packet: Packet, server_ip: str) -> bool:
        if 'ip' in packet:
            return packet.ip.dst.get_default_value() == server_ip
        if 'ipv6' in packet:
            return packet.ipv6.dst.get_default_value() == server_ip

    def get_session_label(self, session: List[Packet]) -> Dict[str, Union[str, bool]]:
        label = 'label unknown'
        missing = False
        if self.label_dataframe is not None:
            # Iterate over all packets until we hit the client hello, then save its random
            session_client_hello_random = ""
            for packet in session:
                if 'TLS' in packet and packet.tls.get('handshake') and \
                        packet.tls.get('handshake').get_default_value() == 'Handshake Protocol: Client Hello':
                    # We got a TLS Client Hello, extract the randomness from it
                    session_client_hello_random = packet.tls.get('handshake_random').get_default_value()

                if 'SSL' in packet and packet.ssl.get('handshake') and \
                        packet.ssl.get('handshake').get_default_value() == 'Handshake Protocol: Client Hello':
                    # We got a SSL Client Hello, extract the randomness from it
                    session_client_hello_random = packet.ssl.get('handshake_random').get_default_value()
            # Match the session random to the randoms in the label dataframe
            matching_label = self.label_dataframe.loc[self.label_dataframe['client_hello_random'] == session_client_hello_random]
            if len(matching_label) > 0:
                label = matching_label['label'].iloc[0]
                if 'skipped_ccs_fin' in matching_label:
                    missing = matching_label['skipped_ccs_fin'].iloc[0]
            else:
                print(f'Warning: No matching label found for randomness "{session_client_hello_random}"')

        return {'label': label, 'missing_ccs_fin': missing}

    @staticmethod
    def get_server_ip(session: List[Packet]) -> str:
        for packet in session:
            if 'TLS' in packet and packet.tls.get('handshake') and \
                    packet.tls.get('handshake').get_default_value() == 'Handshake Protocol: Client Key Exchange':
                # We got a TLS Client Key Exchange, extract the server IP from it
                if 'ip' in packet:
                    return packet.ip.dst.get_default_value()
                if 'ipv6' in packet:
                    return packet.ipv6.dst.get_default_value()

            if 'SSL' in packet and packet.ssl.get('handshake') and \
                    packet.ssl.get('handshake').get_default_value() == 'Handshake Protocol: Client Key Exchange':
                # We got a SLL Client Key Exchange, extract the server IP from it
                if 'ip' in packet:
                    return packet.ip.dst.get_default_value()
                if 'ipv6' in packet:
                    return packet.ipv6.dst.get_default_value()
        return ''

    def extract_packet_values(self, response_packet: Packet, machine_packet_name: str, human_packet_name: str) -> (
            dict, Dict[str, str]):
        # Every packet is TCP, extract its values
        packet_extracted_values, packet_column_names = self.extract_layer_values(response_packet.tcp, machine_packet_name,
                                                                            human_packet_name)
        if 'TLS' in response_packet or 'SSL' in response_packet:
            # If packet contains TLS, also extract that
            layer_to_extract = response_packet.tls if 'TLS' in response_packet else response_packet.ssl
            tls_extracted_values, tls_column_names = self.extract_layer_values(layer_to_extract, machine_packet_name,
                                                                          human_packet_name)
            packet_extracted_values.update(tls_extracted_values)
            packet_column_names.update(tls_column_names)
        return packet_extracted_values, packet_column_names

    def extract_layer_values(self, layer: Layer, machine_packet_name: str, human_packet_name: str) -> (dict, Dict[str, str]):
        # Ignore these, they are not interesting for the ML algorithm
            #    field_ignorelist = ['_raw', '.analysis.', 'time']
        extracted_fields = {}
        extracted_names = {}
        # noinspection PyProtectedMember
        for machine_field_name, field_value in layer._all_fields.items():
            if layer.layer_name in machine_field_name:
                try:
                    extracted_field_value = int(field_value.get_default_value())
                except ValueError:
                    try:
                        extracted_field_value = float(field_value.get_default_value())
                    except ValueError:
                        # It's a string
                        extracted_field_value = field_value.get_default_value()

                if not type(extracted_field_value) is str:
                    # We have a numeric value we can use
                    if not any(_ in machine_field_name for _ in field_ignorelist):
                        # The field name does not match anything in the ignore list
                        key = f'{machine_packet_name}:{machine_field_name}'
                        extracted_fields[key] = extracted_field_value
                        # Create a human-readable description of this item for interpretability purposes
                        extracted_names[key] = self.get_human_description(field_value, layer, machine_field_name,
                                                                     human_packet_name)
        return extracted_fields, extracted_names

    @staticmethod
    def get_human_description(field_value: LayerFieldsContainer, layer: Layer, machine_field_name: str,
                              human_packet_name: str) -> str:
        human_field_name = field_value.showname_key
        if type(human_field_name) is not str:
            # This entry unfortunately does not have a human-readable name, fall back to the machine-readable one
            human_field_name = machine_field_name
        if layer.layer_name.upper() in human_field_name:
            return f'{human_field_name} of the {human_packet_name}'
        else:
            return f'{layer.layer_name.upper()} {human_field_name} of the {human_packet_name}'

    def extract_session_features(self, session: List[Packet]) -> (dict, dict):
        session_features = {}
        session_column_names = {}
        server_ip = self.get_server_ip(session)
        state_machine = StateMachine()

        for packet in session:
            # Transition to a new state if necessary
            if self.is_from_server(packet, server_ip):
                state_machine.transition_on_server_packet(packet)
            if self.is_from_client(packet, server_ip):
                state_machine.transition_on_client_packet(packet)

            # Process a server packet
            if not state_machine.state == State.WAIT and self.is_from_server(packet, server_ip):
                # Determine the packet name
                machine_packet_name = f'{state_machine.state.name}{state_machine.extracted_packet_counter}'
                human_packet_name = f'{num2words(state_machine.extracted_packet_counter + 1, True)} {state_machine.state.value}'
                packet_features, packet_column_names = self.extract_packet_values(packet, machine_packet_name, human_packet_name)

                # Add another meta-feature for the order of the messages
                packet_features[f'{machine_packet_name}:order'] = state_machine.received_packet_counter
                packet_column_names[f'{machine_packet_name}:order'] = \
                    f'Message order of the {human_packet_name} within the server responses'

                session_features.update(packet_features)
                session_column_names.update(packet_column_names)

                # Increment the counters in the state machine
                state_machine.increment_counter()

        return session_features, session_column_names

    def extract_capture_features(self) -> (pandas.DataFrame, pandas.DataFrame):
        print('Starting feature extraction')

        capture_labeled_features = []
        capture_column_names = {}
        for index, tcp_session in self.iterable_capture:
            session_label = self.get_session_label(tcp_session)
            session_features, session_column_names = self.extract_session_features(tcp_session)
            # Trick for merging the dicts
            session_label.update(session_features)
            session_labeled_features = session_label

            if len(session_features) < 3:
                print(f'Ignoring session {index} containing no TLS key exchange')
            else:
                capture_labeled_features.append(session_labeled_features)
                capture_column_names.update(session_column_names)

        features_dataframe = pandas.DataFrame(capture_labeled_features)
        column_names_dataframe = pandas.DataFrame(capture_column_names.items(), columns=['machine', 'human'])
        print(f'Finished feature extraction')
        return features_dataframe, column_names_dataframe


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--folder', '-f', help='Folder that contains the input files Packets.pcap and Client Requests.csv '
                                               'and that the output files will be written to')
    args = parser.parse_args()
    extractor = FeatureExtractor(f'{args.folder}/Packets.pcap', f'{args.folder}/Client Requests.csv')
    feature_dataframe, column_name_dataframe = extractor.extract_capture_features()
    feature_dataframe.to_csv(f'{args.folder}/Features.csv')
    feature_dataframe.to_excel(f'{args.folder}/Features.xlsx')
    column_name_dataframe.to_csv(f'{args.folder}/Feature Names.csv')
    column_name_dataframe.to_excel(f'{args.folder}/Feature Names.xlsx')

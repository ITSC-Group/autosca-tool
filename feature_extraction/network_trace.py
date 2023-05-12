from typing import List, Optional, Dict, Union

from pyshark.capture.capture import Capture, TSharkCrashException
import pyshark
from pyshark.packet.packet import Packet


class IterableCapture:
    def __init__(self, capture_path: str, slice_size: int = 10000):
        self.capture_path = capture_path
        print(f'Loading {self.capture_path}')
        self.slice_size = slice_size
        self.next_slice_start = 0
        self.slice_iterator = None
        self.load_next_slice()
        self.current_tcp_index = 0

    def __iter__(self):
        return self

    def __next__(self):
        try:
            result = (self.current_tcp_index, next(self.slice_iterator))
        except StopIteration:
            # The slice is exhausted, load the next
            self.load_next_slice()
            result = (self.current_tcp_index, next(self.slice_iterator))
        self.current_tcp_index = self.current_tcp_index + 1
        return result

    def load_next_slice(self):
        slice_start = self.next_slice_start
        slice_end = slice_start + self.slice_size
        wireshark_display_filter = f'tcp.stream >= {slice_start} and tcp.stream < {slice_end}'
        print(f'Processing {wireshark_display_filter}')
        try:
            slice_capture = pyshark.FileCapture(
                self.capture_path,
                keep_packets=False,
                display_filter=wireshark_display_filter)
        except TSharkCrashException:
            print(f'Warning: TShark returned a non-zero returncode and might have crashed. '
                  f'When running docker, this happens because of asyncio and is no cause for concern.')
            slice_capture = [].__iter__()
        self.next_slice_start = slice_end
        self.slice_iterator = self.splice_sessions(slice_capture)


    def splice_sessions(self, packet_capture: Capture):
        """
        Splits a single continuous capture file into the TCP sessions contained in it.
        Non-TCP packets are ignored.

        :param packet_capture: The pyshark capture to analyze
        :return: A list of TCP sessions, each a list of Packets
        """
        session_list = []
        current_slice_start = self.next_slice_start - self.slice_size
        for packet in packet_capture:
            if 'TCP' in packet:
                session_index = int(packet.tcp.stream.get_default_value()) - current_slice_start
                if session_index >= len(session_list):
                    # New session, grow the list
                    session_list.append([packet])
                else:
                    session_list[session_index].append(packet)
        packet_capture.close()
        return session_list.__iter__()

from pyshark.packet.packet import Packet
from enum import Enum


class State(Enum):
    WAIT = 'message received in the unreachable waiting state'
    CKE = 'Client Key Exchange TCP Acknowledgement'
    CCS = 'Change Cipher Spec TCP Acknowledgement'
    FIN = 'Client Finished TCP Acknowledgement'
    TLS = 'TLS Alert'
    DISC = 'TCP Disconnect'


class StateMachine:
    def __init__(self):
        self.state = State.WAIT
        self.extracted_packet_counter = 0
        self.received_packet_counter = 0

    def transition_on_client_packet(self, packet: Packet):
        # Processsing a Client Key Exchange
        if 'TLS' in packet \
                and packet.tls.get('handshake') \
                and packet.tls.get('handshake').get_default_value() == 'Handshake Protocol: Client Key Exchange':
            self.try_transition(State.CKE)
        if 'SSL' in packet \
                and packet.ssl.get('handshake') \
                and packet.ssl.get('handshake').get_default_value() == 'Handshake Protocol: Client Key Exchange':
            self.try_transition(State.CKE)

        # Processing a Change Cipher Spec
        if 'TLS' in packet \
                and packet.tls.get('change_cipher_spec') \
                and packet.tls.get('change_cipher_spec').get_default_value() \
                == 'Change Cipher Spec Message':
            self.try_transition(State.CCS)
        if 'SSL' in packet \
                and packet.ssl.get('change_cipher_spec') \
                and packet.ssl.get('change_cipher_spec').get_default_value() \
                == 'Change Cipher Spec Message':
            self.try_transition(State.CCS)

        # Processing a Client Finished
        if 'TLS' in packet \
                and packet.tls.get('handshake') \
                and packet.tls.get('handshake').get_default_value() \
                == 'Handshake Protocol: Encrypted Handshake Message':
            self.try_transition(State.FIN)
        if 'SSL' in packet \
                and packet.ssl.get('handshake') \
                and packet.ssl.get('handshake').get_default_value() \
                == 'Handshake Protocol: Encrypted Handshake Message':
            self.try_transition(State.FIN)

        # Processing a TCP disconnect
        if 'TCP' in packet \
                and (packet.tcp.get('flags.fin').get_default_value() == "1"
                     or packet.tcp.get('flags.res').get_default_value() == "1"
                     or packet.tcp.get('flags.reset').get_default_value() == "1"):
            self.try_transition(State.DISC)

    def transition_on_server_packet(self, packet: Packet):
        # Processing a TLS alert (or other TLS messages received after the CKE)
        if self.state != State.WAIT \
                and ('TLS' in packet
                     or 'SSL' in packet):
            self.try_transition(State.TLS)

        # Processing a TCP disconnect
        if 'TCP' in packet \
                and (packet.tcp.get('flags.fin').get_default_value() == "1"
                     or packet.tcp.get('flags.res').get_default_value() == "1"
                     or packet.tcp.get('flags.reset').get_default_value() == "1"):
            self.try_transition(State.DISC)

    def try_transition(self, new_state: State):
        if self.state == State.DISC:
            # Once the disconnection started, we don't leave this state, regardless of other TLS messages
            return

        if not self.state == new_state:
            # Reset the state counter when transitioning to a new state
            self.extracted_packet_counter = 0
        self.state = new_state

    def increment_counter(self):
        self.extracted_packet_counter = self.extracted_packet_counter + 1
        self.received_packet_counter = self.received_packet_counter + 1

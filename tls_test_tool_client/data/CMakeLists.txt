cmake_minimum_required(VERSION 3.4.0)

install(FILES
		test_server_ec_certificate.pem
		test_server_ec_private_key.pem
		test_server_rsa_certificate.pem
		test_server_rsa_private_key.pem		
	DESTINATION data
)

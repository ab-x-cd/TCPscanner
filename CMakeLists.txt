cmake_minimum_required(VERSION 3.10)
project(TCPScanner)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tcp_scanner main.cpp)

enable_testing()

# Test de base - compilation et exécution
add_test(NAME basic_functionality COMMAND tcp_scanner 127.0.0.1 22 22)

# Test avec IP invalide (devrait échouer)
add_test(NAME invalid_ip_test COMMAND tcp_scanner 999.999.999.999 80 80)
set_tests_properties(invalid_ip_test PROPERTIES WILL_FAIL TRUE)

# Test sans arguments (devrait échouer)
add_test(NAME no_arguments_test COMMAND tcp_scanner)
set_tests_properties(no_arguments_test PROPERTIES WILL_FAIL TRUE)

# Test avec arguments insuffisants (devrait échouer)
add_test(NAME insufficient_args_test COMMAND tcp_scanner 127.0.0.1 80)
set_tests_properties(insufficient_args_test PROPERTIES WILL_FAIL TRUE)

# Test avec plage de ports
add_test(NAME port_range_test COMMAND tcp_scanner 127.0.0.1 80 85)

# Test avec port unique
add_test(NAME single_port_test COMMAND tcp_scanner 127.0.0.1 443 443)

# Test de performance - plusieurs ports
add_test(NAME performance_test COMMAND tcp_scanner 127.0.0.1 12340 12349)
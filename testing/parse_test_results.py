import json
import sys

def parse_test_results(file_path):
    """
    Parse the test results of an nrtest json
    """
    with open(file_path, 'r') as f:
        data = json.load(f)
    
    total_tests = len(data['Tests'])
    passed_tests = sum(1 for test in data['Tests'] if test['passed'])
    failed_tests = total_tests - passed_tests
    
    return passed_tests, failed_tests

if __name__ == "__main__":
    file_path = sys.argv[1]
    passed, failed = parse_test_results(file_path)
    
    # Syntax below is used for github actions testing
    print(f"passed_tests={passed}")
    print(f"failed_tests={failed}")
    print(f"total_tests={passed + failed}")
    print(f"all_tests_passed={failed == 0}")

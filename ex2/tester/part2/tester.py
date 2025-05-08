import json
import subprocess

FILE_NAME = 'tests.json'
PROGRAM_NAME = './file_processor'
all_passed = True
def show_diff(expected, actual, label):
    print(f"\n--- Expected {label}:")
    print(expected)
    print(f"\n+++ Actual {label}:")
    print(actual)

with open(FILE_NAME) as f:
    tests = json.load(f)

for test_index, test in enumerate(tests, start=1):
    with open('data.txt', 'w') as f:
        f.write(test['data'])
    with open('requests.txt', 'w') as f:
        f.write(test['requests'])

    subprocess.run([PROGRAM_NAME, 'data.txt', 'requests.txt'])

    with open('data.txt', 'r') as f:
        data_changed = f.read()
    with open('read_results.txt', 'r') as f:
        read_results_data = f.read().rstrip('\n')

    data_match = data_changed == test['data_changed']
    results_match = read_results_data == test['read_results']

    if data_match and results_match:
        print(f"✅ Test {test_index} passed")
    else:
        all_passed = False
        print(f"\n❌ Test {test_index} failed")
        if not data_match:
            print("\n[data.txt mismatch]")
            show_diff(test['data_changed'], data_changed, 'data.txt')

        if not results_match:
            print("\n[read_results.txt mismatch]")
            show_diff(test['read_results'], read_results_data, 'read_results.txt')
    print('-' * 60)

if all_passed:
    print(r"""    .--.
   |o_o |
   |\_/ |
  //   \ \
 (|     | )
/'\_   _/`\
\___)=(___/
""")
    
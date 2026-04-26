import os
import subprocess
import sys

def main():
    test_dir = os.path.dirname(os.path.abspath(__file__))
    input_dir = os.path.join(test_dir, 'inputs')
    expected_dir = os.path.join(test_dir, 'expected')
    
    compiler_path = os.path.join(test_dir, '..', 'build', 'novac.exe')
    if not os.path.exists(compiler_path):
        compiler_path = os.path.join(test_dir, '..', 'build', 'novac')
    
    if not os.path.exists(compiler_path):
        print(f"Compiler not found at {compiler_path}. Please build it first.")
        sys.exit(1)

    failed = 0
    passed = 0

    for filename in os.listdir(input_dir):
        if filename.endswith('.nv'):
            test_name = filename[:-3]
            input_file = os.path.join(input_dir, filename)
            expected_file = os.path.join(expected_dir, f"{test_name}.txt")
            exe_file = os.path.join(test_dir, f"{test_name}.exe")
            
            print(f"Running test: {test_name} ... ", end="")
            
            # Read expected return code
            expected_code = 0
            if os.path.exists(expected_file):
                with open(expected_file, 'r') as f:
                    try:
                        expected_code = int(f.read().strip())
                    except ValueError:
                        pass
            
            # Compile
            compile_res = subprocess.run([compiler_path, input_file, '-o', exe_file], capture_output=True, text=True)
            if compile_res.returncode != 0:
                print("FAILED (Compilation error)")
                print(compile_res.stderr)
                failed += 1
                continue
                
            # Run
            run_res = subprocess.run([exe_file], capture_output=True)
            if run_res.returncode == expected_code:
                print("PASSED")
                passed += 1
            else:
                print(f"FAILED (Expected exit code {expected_code}, got {run_res.returncode})")
                failed += 1
                
            # Cleanup
            if os.path.exists(exe_file):
                os.remove(exe_file)
            if os.path.exists('output.ll'):
                os.remove('output.ll')
            if os.path.exists('output.o'):
                os.remove('output.o')

    print(f"\nTests completed: {passed} passed, {failed} failed.")
    sys.exit(failed)

if __name__ == '__main__':
    main()

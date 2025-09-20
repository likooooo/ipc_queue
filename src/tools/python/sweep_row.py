#!/usr/bin/env python3
import argparse
import sys
import ast

"""
处理一行参数数据，将其转换为一个包含正确类型的元组。
"""
def process_data(line):
    line = line.strip()
    if not line: return []

    str_params = line.split('\t')
    
    converted_params = []
    for p in str_params:
        try:
            # 使用 ast.literal_eval 安全地评估字符串，将其转换为原始类型
            # 这可以处理数字（整数和浮点数）、布尔值、None等
            converted_p = ast.literal_eval(p)
            converted_params.append(converted_p)
        except (ValueError, SyntaxError):
            # 如果 ast.literal_eval 失败，说明它可能是一个简单的字符串
            # 此时，我们将其作为字符串处理
            converted_params.append(p)
    return converted_params

def main():
    def prase_cmd():
        parser = argparse.ArgumentParser(
            description="A script to process piped data and execute an external command."
        )
        
        parser.add_argument(
            '--cmd', 
            type=str, 
            required=False,
            default="",
            help="The external command to execute for each line of data. E.g., 'your_program'"
        )
        parser.add_argument(
            '--securty_mode', 
            type=int, 
            required=False,
            default=False,
            help="The external command to execute for each line of data. E.g., 'your_program'"
        )
        parser.add_argument(
            '--queue', 
            type=str, 
            required=False,
            default="shell",
            help="The communication protocol of the specified program"
        )
        return parser.parse_known_args()
    
    def run_cmd(command, sweep_row, security_mode = False):
        if security_mode:
            import subprocess
            try:
                # 使用 f-string 创建命令行模板，但要确保它是安全的
                # 注意：此处假设 command_template 不包含需要进一步评估的参数，
                # 而是直接提供了命令名
                command_args = command.split() + [str(p) for p in sweep_row]
                result = subprocess.run(command_args, capture_output=True, text=True, check=True)
                print(result.stdout, end="")
                if result.stderr:
                    print(result.stderr)
                    exit(1)
            except subprocess.CalledProcessError as e:
                print(f"  Error executing command: {e}", file=sys.stderr)
            except FileNotFoundError:
                print(f"  Error: Command '{command_args[0]}' not found.", file=sys.stderr)
        else:
            import os
            cmd = ' '.join([command] + [str(s) for s in sweep_row])
            os.system(cmd)

    args, unknown  = prase_cmd()
    try:
        for line in sys.stdin:
            converted_params = process_data(' '.join(unknown) + ' ' + line)
            if len(converted_params):
                if "shell" == args.queue:
                    run_cmd(args.cmd, converted_params, args.securty_mode)
                elif "ipc" == args.queue:
                    # push
                    run_cmd(args.cmd, unknown)
                    # pop
                else:
                    print(f"unfinished code in queue type={args.queue}", file=sys.stderr)
                    sys.exit(1)
                    # send
                    # receive
                    # run_cmd(args.cmd, [])
    except Exception as e:
        print(f"An error occurred: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()
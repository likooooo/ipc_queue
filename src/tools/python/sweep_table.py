#!/usr/bin/env python3
import numpy as np
import argparse
import itertools
import ast
import sys

def main():
    check_len = lambda x :  len(x) if x is not None else 0
   
    def parse_args():
        parser = argparse.ArgumentParser(
            description="A script to perform parameter sweeps with multiple parameters."
        )
        
        parser.add_argument(
            '--start', 
            type=float, 
            nargs='*',
            help="The start value(s) of the parameter sweep. Separate values with spaces."
        )
        
        parser.add_argument(
            '--end', 
            type=float, 
            nargs='*',
            help="The end value(s) of the parameter sweep. Separate values with spaces."
        )
        
        parser.add_argument(
            '--size', 
            type=int, 
            nargs='*',
            help="The number of step(s) in the sweep. Separate values with spaces."
        )
        
        def parse_nominal(arg_string):
            try:
                return ast.literal_eval(arg_string)
            except (ValueError, SyntaxError) as e:
                raise argparse.ArgumentTypeError(f"Invalid format for nominal parameter: {e} \n\t\targ_string={arg_string}")

        parser.add_argument(
            '--nominal', 
            type=parse_nominal,
            required=False,
            help="A list of lists for nominal parameters. E.g., '[[a,b],[10,100]]'."
        )

        # 随机采样
        parser.add_argument(
            '--random_sampling',
            type=str,
            required=False,
            default=None,
            help=""
        )
        parser.add_argument(
            '--seed',
            type=int,
            required=False,
            default=4396,
            help=""
        )
        options = ['mu', 'sigma', 'rate','shape', 'scale','a','b','lambda','np']
        for o in options:
            parser.add_argument(
                '--' + o,
                type=float,
                required=False,
                default=None,
                help="optional-params for random sampling"
            )

        args = parser.parse_args()
        if not (check_len(args.start) == check_len(args.end) == check_len(args.size)):
            print("Error: --start, --end, and --size must have the same number of elements.", file=sys.stderr)
            exit(1)
            
        if None is not args.random_sampling : np.random.seed(args.seed)
        return args
    
    """
    生成指定范围 [start, end] 内的随机数，支持多种分布。

    Args:
        start (float): 随机数的起始点。
        end (float): 随机数的终止点。
        distribution (str): 指定随机数分布类型。
        **kwargs: 额外的分布参数。

    Returns:
        float: 生成的随机数。
    
    Raises:
        ValueError: 如果分布类型不支持。
    """
    def get_random_number(args, start: float, end: float, distribution: str = 'uniform') -> float:

        if start > end:
            start, end = end, start
        
        # 连续分布
        if distribution == 'uniform':
            return np.random.uniform(start, end)
        elif distribution in ['gauss', 'normal']:
            mu, sigma = args.mu, args.sigma
            if None is mu: mu = (start + end) / 2
            if None is sigma: sigma = (end - start) / 4
            return np.random.normal(mu, sigma)
        elif distribution == 'exponential':
            rate = args.rate
            if None is rate: rate = 1
            return np.random.exponential(1/rate) + start # 注意：需要调整范围
        elif distribution == 'gamma':
            shape, scale = args.shape, args.scale
            if None is shape: shape = 2
            if None is scale: scale = 1
            return np.random.gamma(shape, scale)
        elif distribution == 'beta':
            a,b = args.a, args.b
            if None is a: a = 2
            if None is b: b = 5
            return np.random.beta(a, b) * (end - start) + start # 将 [0,1] 映射到 [start,end]

        # 离散分布
        elif distribution == 'poisson':
            lam = args.lam
            if None is lam: lam = 1
            return np.random.poisson(lam)
        elif distribution == 'binomial':
            n,p = args.n, args.p
            if None is n: n = 10
            if None is p: p = 0.5
            return np.random.binomial(n, p)
        else:
            raise ValueError(f"不支持的分布类型: {distribution}")


    args = parse_args()
    all_sweep_values = []
    if 0 < check_len(args.start):
        def uniform_grid():
            step = (end - start) / (size - 1) if size > 1 else 0
            return [start + i * step for i in range(size)]
        
        def random_grid():
            return [get_random_number(args, start, end, args.random_sampling) for i in range(size)]
            
        for start, end, size in zip(args.start, args.end, args.size):

            if size <= 0:
                print(f"Error: All sizes must be positive integers. Found: {size}", file=sys.stderr)
                return
            
            all_sweep_values.append([uniform_grid, random_grid][int(None is not args.random_sampling)]())
        
    all_params = all_sweep_values
    if args.nominal:
        if isinstance(args.nominal[0], list):
            all_params = args.nominal + all_params
        else:
            all_params = [args.nominal] + all_params
    
    all_combinations = itertools.product(*all_params)
    
    for combo in all_combinations:
        print('\t'.join(map(str, combo)))

if __name__ == "__main__":
    main()
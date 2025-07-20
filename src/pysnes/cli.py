import os
import time

import click

from .env import SnesEnv


@click.command()
@click.argument("rom")
@click.option("--human", "mode", flag_value="human", default=True)
@click.option("--random", "mode", flag_value="random")
@click.version_option()
def cli(rom: os.PathLike, mode: str):
    """Main CLI entrypoint for pysnes"""
    env = SnesEnv(rom)
    obs = env.reset()

    if mode == "human":
        # Human mode: requires a keyboard listener
        print("Human mode is not fully implemented yet.")
        print("Use random mode to see the emulator in action.")

    elif mode == "random":
        done = False
        while not done:
            action = env.action_space.sample()
            obs, reward, done, info = env.step(action)
            env.render()
            time.sleep(1 / 60)

    env.close()

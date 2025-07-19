import click
import gymnasium
import os


@click.command()
@click.argument('rom')
@click.option('--human', 'mode', flag_value='human', default=True)
@click.option('--random', 'mode', flag_value='random')
@click.version_option()
def cli(rom: os.PathLike):
    """Main CLI entrypoint for pysnes"""
    print("Hello from pysnes!")
    
@click.command()
def human(gym: gymnasium.Env):
    """
    Play the environment using keyboard as a human.
    
    Args:
        env: the initialized gym environment to play
        
    Returns:
        None
    """

    # ensure the observation space is a box of pixels
    assert isinstance(gym.observation_space, gym.spaces.Box), "Observation space must be a box of pixels"
    # ensure the observation space is either B&W pixels or RGB Pixels
    obs_s = gym.observation_space
    is_bw = len(obs_s.shape) == 2
    is_rgb = len(obs_s.shape) == 3 and obs_s.shape[2] in [1, 3]
    assert is_bw or is_rgb, "Observation space must be a box of pixels"
    
    # get keyboard mapping
    if hasattr(gym, "get_keys_to_action"):
        keys_to_action = gym.get_keys_to_action()
    elif hasattr(gym.unwrapped, "get_keys_to_action"):
        keys_to_action = gym.unwrapped.get_keys_to_action()
    else:
        raise ValueError("Environment does not have a get_keys_to_action method")
    
    # TODO: create the image viewer
    viewer = None
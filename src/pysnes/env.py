import os

import gymnasium as gym
import numpy as np
from gymnasium import spaces


class SnesEnv(gym.Env):
    metadata = {"render.modes": ["human"], "render_fps": 60}

    def __init__(self, rom_path):
        super(SnesEnv, self).__init__()
        from pysnes.snes import SNES
        self.snes = SNES()
        self.snes.insert_cartridge(rom_path)
        self.snes.power_on()

        # 8 buttons for the controller
        self.action_space = spaces.Discrete(2**8)
        # The screen is 256x240 with 32-bit color
        self.observation_space = spaces.Box(
            low=0, high=255, shape=(240, 256, 4), dtype=np.uint32
        )
        self.viewer = None

    def step(self, action):
        self.snes.set_controller_state(1, action)
        self.snes.step()
        obs = self.snes.get_screen()
        reward = 1.0  # Implement your reward function here
        done = False  # Implement your termination condition here
        return obs, reward, done, {}

    def reset(self):
        self.snes.reset()
        return self.snes.get_screen()

    def render(self, mode="human"):
        if self.viewer is None:
            from gymnasium.envs.classic_control import rendering

            self.viewer = rendering.SimpleImageViewer()
        img = self.snes.get_screen()
        self.viewer.imshow(img)
        return self.viewer.isopen

    def close(self):
        if self.viewer:
            self.viewer.close()

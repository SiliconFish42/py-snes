import sys
import os

# Add the build directory to the path so we can import pysnes_cpp
# Assuming typical build structure, but might need adjustment
sys.path.append(os.path.abspath("build/lib.win-amd64-cpython-310")) 
# Also try just local build dir
sys.path.append(os.path.abspath("build"))

try:
    from pysnes import pysnes_cpp
except ImportError:
    try:
        import pysnes_cpp
    except ImportError:
        print("Could not import pysnes_cpp via package or direct. check path.")
        import sys
        print(sys.path)
        sys.exit(1)

import numpy as np

def verify_framebuffer():
    print("Initializing SNES...")
    snes = pysnes_cpp.SNES()
    
    # Power on to initialize state
    snes.power_on()
    
    print("Getting screen buffer...")
    screen = snes.get_screen()
    
    print(f"Type: {type(screen)}")
    print(f"Shape: {screen.shape}")
    print(f"Dtype: {screen.dtype}")
    
    if screen.shape != (224, 256):
        print("FAIL: Incorrect shape")
        return False
        
    if screen.dtype != np.uint32:
        print("FAIL: Incorrect dtype")
        return False
        
    # Check if it's writeable/view test
    # We can't easily write to it and check C++ side without a reverse binding, 
    # but we can check if it's a valid numpy array.
    
    # Let's try to get RGB buffer too
    print("Getting RGB buffer...")
    rgb = snes.get_framebuffer_rgb()
    print(f"RGB Shape: {rgb.shape}")
    
    if rgb.shape != (224, 256, 3):
        print("FAIL: Incorrect RGB shape")
        return False
        
    print("SUCCESS: Framebuffer bindings verified.")
    return True

if __name__ == "__main__":
    if verify_framebuffer():
        sys.exit(0)
    else:
        sys.exit(1)

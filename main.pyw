import time
import json
import requests
import subprocess

# ESP8266 Web Server URL
ESP_URL = "http://mongyro.local/data"

# Define rotation thresholds (tweak if needed)
THRESHOLDS = {
    "normal": (0, 0),    # Upright
    "right": (180, 90),   # 90° Clockwise
}

ALLOWANCE = 30  # Allowance for threshold

REFRESH_INTERVAL = 10  # Refresh interval in seconds

# Windows Display Rotation using PowerShell command
def rotate_display(orientation):
    ROTATION_MAP = {
        "normal": 1,     # Normal orientation
        "right": 2,      # 90° Clockwise
    }

    rotation_value = ROTATION_MAP.get(orientation, 1)  # Default to "normal"
    monitor_number = 2  # Change this to your secondary monitor's number

    # PowerShell command to rotate specific monitor
    powershell_command = f"Set-DisplayRotation {monitor_number} {rotation_value}"

    try:
        subprocess.run(["powershell", "-Command", powershell_command], shell=True, check=True)
        print(f"Rotated monitor {monitor_number} to {orientation} ({rotation_value})")
    except subprocess.CalledProcessError as e:
        print(f"Error executing PowerShell command: {e}")


# # Function to determine monitor orientation
# def get_orientation(pitch, roll):
#     if THRESHOLDS["normal"][0] <= pitch <= THRESHOLDS["normal"][1]:
#         return "normal"
#     elif THRESHOLDS["right"][0] <= roll <= THRESHOLDS["right"][1]:
#         return "right"
#     elif THRESHOLDS["inverted"][0] <= pitch <= THRESHOLDS["inverted"][1]:
#         return "inverted"
#     elif THRESHOLDS["left"][0] <= roll <= THRESHOLDS["left"][1]:
#         return "left"
#     return "unknown"

def get_orientation(pitch, roll):
    if 50 >= roll:
        return "normal"
    else:
        return "right"

# Main loop to poll ESP8266
def main():
    last_orientation = None

    while True:
        try:
            # Fetch data from ESP8266
            response = requests.get(ESP_URL, timeout=2)
            data = response.json()
            pitch, roll = data["pitch"], data["roll"]

            # Determine orientation
            orientation = get_orientation(pitch, roll)

            # Rotate only if the orientation has changed
            if orientation != last_orientation and orientation != "unknown":
                print(f"New Orientation: {orientation}")
                rotate_display(orientation)
                last_orientation = orientation

        except requests.exceptions.RequestException as e:
            print(f"Error connecting to ESP8266: {e}")

        except json.JSONDecodeError:
            print("Failed to decode JSON response!")

        # Poll every second
        time.sleep(REFRESH_INTERVAL)

if __name__ == "__main__":
    main()

import PySimpleGUI as sg
import serial
import datetime

# Create a serial connection to the Arduino
ser = serial.Serial('COM12', 9600)  # Replace 'COMx' with the correct serial port name

# Create a layout for the UI
layout = []

# Add a title row
layout.append([sg.Text("Floor Monitoring System", size=(30, 1), justification='center', font=("Helvetica", 20))])

# Map floor numbers to labels
floor_labels = ["Basement", "Floor 1", "Floor 2"]

# Add rows for each floor's data with increased vertical gap
for i in range(3):  # Assuming you have 3 floors
    row = [
        sg.Text(f"Floor: {floor_labels[i]}", size=(20, 1)),
        sg.Text("Motion Status: ", key=f"Motion Status: {i}", text_color='black'),
        sg.Text("Temperature: ", key=f"Temperature: {i}", text_color='black'),
    ]

    # Add "Accessed" information for Basement only
    if i == 0:
        row.append(sg.Text("Accessed: ", key=f"Accessed: {i}"))

    layout.append(row)

    # Add additional vertical space between floors
    if i < 2:
        layout.append([sg.Stretch()])

    # Add a vertical separator for space between floors
    if i < 2:
        layout.append([sg.VerticalSeparator()])

# Add a row for the current floor information
layout.append([sg.Text("Current Floor: ", key="-CURRENT_FLOOR-", text_color='black')])

# Add a button to toggle motion detection mode
layout.append([sg.Button("After Hours", key="-TOGGLE_MODE-", enable_events = True)])  # Initial label is "After Hours"

# Create the window with resizable property set to True
window = sg.Window("Floor Monitoring", layout, resizable=True)

# Variables to store the motion detection mode and error counts
motion_detection_mode = True
error_counts = [0, 0, 0]  # One count for each floor

# Flag to track whether motion is currently detected on a specific floor
motion_detected_flags = [False, False, False]

# Flag to track whether temperature is currently below 40 or above 90 on a specific floor
temperature_alert_flags = [False, False, False]

# Create a log file to store access events
log_file = open("access_log.txt", "a")

while True:
    event, values = window.read(timeout=1000)  # Timeout to check for events and update every second
    if event == sg.WIN_CLOSED:
        break

    # Toggle motion detection mode on button click
    if event == "-TOGGLE_MODE-":
        motion_detection_mode = not motion_detection_mode
        # Send a command to the Arduino to toggle motion detection mode
        ser.write(bytes(1))
        ser.write(b'\x01')

        # Change the button label after clicking
        new_label = "After Hours" if motion_detection_mode else "Public Hours"
        window["-TOGGLE_MODE-"].update(text=new_label)

    try:
        # Read data from the Arduino until a newline character is encountered
        data = ser.read_until(b'\n').decode().strip()
        print(f"Raw data: {data}")  # Debugging print statement

        # Check if the data starts with the start marker
        start_marker = "START_MARKER"
        if data.startswith(start_marker):
            # Process the rest of the data
            data = data[len(start_marker):]
            data_list = data.split(',')

            # Update the window with the latest data
            for i in range(3):
                # Update motion status
                motion_detected = int(data_list[i])
                motion_status_key = f"Motion Status: {i}"

                # Flash red text color when motion is detected during after hours
                if motion_detection_mode and motion_detected:
                    if not motion_detected_flags[i]:
                        window[motion_status_key].update(f"Motion Status: {'Motion Detected'}", text_color='red')
                        motion_detected_flags[i] = True
                # Flash red text color when no motion is detected during public hours
                elif not motion_detection_mode and not motion_detected:
                    if not motion_detected_flags[i]:
                        window[motion_status_key].update(f"Motion Status: {'No Motion Detected'}", text_color='red')
                        motion_detected_flags[i] = True
                else:
                    window[motion_status_key].update(f"Motion Status: {'Motion Detected' if motion_detected else 'No Motion Detected'}", text_color='black')
                    motion_detected_flags[i] = False

                # Update temperature
                try:
                    temperature = float(data_list[i + 3]) if len(data_list) > i + 3 else float('nan')
                    temperature_key = f"Temperature: {i}"

                    # Flash red text color when temperature is below 40 or above 90
                    if temperature < 40 or temperature > 90:
                        if not temperature_alert_flags[i]:
                            window[temperature_key].update(f"Temperature: {temperature:.2f} F", text_color='red')
                            temperature_alert_flags[i] = True
                    else:
                        window[temperature_key].update(f"Temperature: {temperature:.2f} F", text_color='black')
                        temperature_alert_flags[i] = False

                except ValueError:
                    window[f"Temperature: {i}"].update(f"Temperature: N/A")

                # Update accessed status for Basement
                if i == 0:
                    try:
                        accessed_status = data_list[i + 7]
                        window[f"Accessed: {i}"].update(f"Accessed: {accessed_status}")
						
                    
                    except IndexError:
                        window[f"Accessed: {i}"].update("Accessed: N/A")

                # Check for errors based on motion detection mode
                if motion_detection_mode:
                    # Count Motion Detected as an error during Motion Detected error checking
                    if motion_detected:
                        error_counts[i] += 1
                else:
                    # Count No Motion Detected as an error during No Motion Detected error checking
                    if not motion_detected:
                        error_counts[i] += 1

            # Update the current floor information
            current_floor = int(data_list[6])
            current_floor_key = "-CURRENT_FLOOR-"

            # Display "Error" when the current floor is 3
            if current_floor == 3:
                window[current_floor_key].update(f"Current Floor: Error", text_color='red')
            else:
                window[current_floor_key].update(f"Current Floor: {current_floor}", text_color='black')

            # Print error counts
            print("Error Counts:", error_counts)

        else:
            # Data does not start with the expected marker, print a warning or handle it as needed
            print("Invalid data format:", data)

    except UnicodeDecodeError:
        print("Error decoding data from Arduino")

# Close the serial connection and the window
ser.close()
window.close()

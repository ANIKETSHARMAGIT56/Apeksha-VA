#!/bin/bash
#!/bin/bash

EXECUTABLE_NAME="your_executable_name"

if pgrep -x "apeksha-server" > /dev/null; then
    echo "The executable is already running."
else
    echo "The executable is not running."
    build/apeksha-server/apeksha-server &
fi


if pgrep -x "apeksha-gui" > /dev/null; then
    echo "The executable is already running."
else
    echo "The executable is not running."
    build/apeksha-gui/apeksha-floaterwindow > /dev/null &
fi


if pgrep -x "apeksha-hotword" > /dev/null; then
    echo "The executable is already running."
else
    echo "The executable is not running."
    build/apeksha-hotword/apeksha-hotword > /dev/null &
fi

wait 
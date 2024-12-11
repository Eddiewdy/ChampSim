#!/bin/bash

# Function to display usage
usage() {
    echo "Usage: $0 [run|build]"
    echo "Options:"
    echo "  run   : Run the Champsim binary with specified arguments"
    echo "  build : Build Champsim using the configuration script"
    exit 1
}

# Check if at least one argument is provided
if [ $# -lt 1 ]; then
    usage
fi

# Parse the argument
case $1 in
    run)
        echo "Preparing to run Champsim..."

        # Define variables
        WARMUP_INSTRUCTIONS=1000000
        SIMULATION_INSTRUCTIONS=3000000
        TRACE_PATH="/mnt/disk/wangyidi/pin-3.22-98547-g7a303a835-gcc-linux/source/tools/MyPinTool_xml/roms_293b_40m"
        OUTPUT_FILE="out/out_roms293b4m_ipstride_1211"

        # Display current settings
        echo "Current Settings:"
        echo "  Warmup Instructions   : $WARMUP_INSTRUCTIONS"
        echo "  Simulation Instructions: $SIMULATION_INSTRUCTIONS"
        echo "  Trace Path            : $TRACE_PATH"
        echo "  Output File           : $OUTPUT_FILE"

        # Confirm settings with the user
        read -p "Please confirm that the above settings are correct (y/n, default: y): " CONFIRM
        CONFIRM=${CONFIRM:-y}

        if [[ "$CONFIRM" != "y" ]]; then
            echo "Aborting run."
            exit 1
        fi

        # Wait for 5 seconds
        # echo "Waiting for 5 seconds..."
        # sleep 5

        # Run the Champsim binary
        echo "Running Champsim..."
        bin/champsim --warmup_instructions $WARMUP_INSTRUCTIONS --simulation_instructions $SIMULATION_INSTRUCTIONS \
        $TRACE_PATH &> $OUTPUT_FILE

        # Check the result of the run
        if [ $? -eq 0 ]; then
            echo "Run completed successfully!"
        else
            echo "Error occurred during run."
        fi
        ;;
    build)
        echo "Building Champsim..."
        ./config.sh champsim_config.json
        make
        if [ $? -eq 0 ]; then
            echo "Build completed successfully!"
        else
            echo "Error occurred during build."
        fi
        ;;
    *)
        usage
        ;;
esac
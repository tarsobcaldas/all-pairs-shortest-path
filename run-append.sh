update_hostname() {
  local l1=$1
  local l2=$2
  local l3=$3
  local l4=$4
  local l5=$5

  {
    echo l101 slots=$1
    echo l102 slots=$2
    echo l103 slots=$3
    echo l104 slots=$4
    echo l105 slots=$5
  } > "hostname"
}

function run_and_append() {
    local graph_size=$1
    local machines=$2
    local processes_per_machine=$3

    # Generate configuration string
    local config="g${graph_size}m${machines}p${processes_per_machine}"

    echo "Starting script"
    if [ ! -f timings.csv ]; then
      # Create CSV file with headers
      echo "Configuration,Run1,Run2,Run3,Run4,...,Run50,TotalTime" > timings.csv
    fi

    echo -n "$config," >> timings.csv

    echo "Running $((processes_per_machine * machines)) processes in ${machines} machines"
    # Loop through 50 runs
    #
    start=$(date +%s.%N)
    for run in {1..50}; do
        # Update the hostname file
        # You need to implement this part based on your needs

        # Run the MPI program and capture timings
        timings=$(mpirun -np $((processes_per_machine * machines)) --hostfile hostname shortest-path -r $graph_size -t |  awk '{print $1}')

        # Append timings to CSV
        echo -n "$timings" >> timings.csv
    done
    end=$(date +%s.%N)

    runtime=$(echo "$end - $start" | bc)
    # Calculate and append total time
    echo "$runtime" >> timings.csv
    echo "Finished in $runtime seconds"
}



# Example runs
update_hostname 1 0 0 0 0
run_and_append 6 1 1
update_hostname 4 0 0 0 0
run_and_append 6 1 4
update_hostname 2 2 0 0 0
run_and_append 6 2 2
update_hostname 1 1 1 1 0
run_and_append 6 4 1
update_hostname 3 3 3 0 0
run_and_append 6 3 3

# # Add total time column headers
# for run in {1..50}; do
#   echo -n "TotalTime${run},"
# done >> timings.csv


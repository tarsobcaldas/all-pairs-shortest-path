function maybe {
  if [ -z "$1" ]; then
    echo 0
  else
    echo "$1" 
  fi
}

update_hostname() {
  {
    echo "l201 slots=$1"
    echo "l202 slots=$(maybe $2)"
    echo "l203 slots=$(maybe $3)"
    echo "l204 slots=$(maybe $4)"
    echo "l205 slots=$(maybe $5)"
    echo "l206 slots=$(maybe $6)"
    echo "l207 slots=$(maybe $7)"
    echo "l208 slots=$(maybe $8)"
    echo "l209 slots=$(maybe $9)"
  } > "hostname"
}

function run_and_append() {
    local graph_size=$1
    # local machine1=$2
    # local machine2=$3
    local machines=$2
    local processes_per_machine=$3
    local total_processes=$((processes_per_machine*machines))
    # local total_processes=$machine1+$machine2

    # Generate configuration string
    local config="g${graph_size}m${machines}p${total_processes}"

    echo "Starting script"
    if [ ! -f timings1200.csv ]; then
      # Create CSV file with headers
      echo "Configuration,Run1,Run2,Run3,Run4,Run5,Run6,Run7,Run8,Run9,Run10,Run11,Run12,Run13,Run14,Run15,Run16,Run17,Run18,Run19,Run20,Run21,Run22,Run23,Run24,Run25,Run26,Run27,Run28,Run29,Run30,Run31,Run32,Run33,Run34,Run35,Run36,Run37,Run38,Run39,Run40,Run41,Run42,Run43,Run44,Run45,Run46,Run47,Run48,Run49,Run50,TotalTime" > timings1200.csv
    fi

    echo -n "$config," >> timings1200.csv

    echo "Running ${total_processes} processes in ${machines} machines"
    # Loop through 50 runs
    #
    start=$(date +%s.%N)
    for run in {1..50}; do
        # Update the hostname file
        # You need to implement this part based on your needs

        # Run the MPI program and capture timings
        timings=$(mpirun -np ${total_processes} --hostfile hostname shortest-path -r $graph_size -t |  awk '{print $1}')

        # Append timings to CSV
        echo -n "$timings" >> timings1200.csv
    done
    end=$(date +%s.%N)

    runtime=$(echo "$end - $start" | bc)
    # Calculate and append total time
    echo "$runtime" >> timings1200.csv
    echo "Finished in $runtime seconds"
}



function run_and_append2() {
    local graph_size=$1
    local machine1=$2
    local machine2=$3
    local machine3=$4
    local machine4=$5
    local machines=$6
    # local machine5=$3
    local total_processes=$(($machine1+$machine2+$machine3+$machine4))

    echo "Starting script"
    if [ ! -f timings1200.csv ]; then
      # Create CSV file with headers
      echo "Configuration,Run1,Run2,Run3,Run4,Run5,Run6,Run7,Run8,Run9,Run10,Run11,Run12,Run13,Run14,Run15,Run16,Run17,Run18,Run19,Run20,Run21,Run22,Run23,Run24,Run25,Run26,Run27,Run28,Run29,Run30,Run31,Run32,Run33,Run34,Run35,Run36,Run37,Run38,Run39,Run40,Run41,Run42,Run43,Run44,Run45,Run46,Run47,Run48,Run49,Run50,TotalTime" > timings1200.csv
    fi

    # Generate configuration string
    local config="g${graph_size}m${machines}p${total_processes}u"

    echo -n "$config," >> timings1200.csv

    echo "Running ${total_processes} processes in ${machines} machines"
    # Loop through 50 runs
    #
    start=$(date +%s.%N)
    for run in {1..50}; do
        # Update the hostname file
        # You need to implement this part based on your needs

        # Run the MPI program and capture timings
        timings=$(mpirun -np ${total_processes} --hostfile hostname shortest-path -r $graph_size -t |  awk '{print $1}')

        # Append timings to CSV
        echo -n "$timings" >> timings1200.csv
    done
    end=$(date +%s.%N)

    runtime=$(echo "$end - $start" | bc)
    # Calculate and append total time
    echo "$runtime" >> timings1200.csv
    echo "Finished in $runtime seconds"
}



update_hostname 1
run_and_append 1200 1 1

update_hostname 4 0 0 0 0 0 0 0 0 0 
run_and_append 1200 1 4

update_hostname 2 2 0 0 0 0 0 0 0 0 
run_and_append 1200 2 2

update_hostname 1 1 1 1 0 0 0 0 0 0 
run_and_append 1200 4 1

update_hostname 3 3 3 0 0 0 0 0 0 0 
run_and_append 1200 3 3

update_hostname 4 5 0 0 0 0 0 0 0 0 
run_and_append2 1200 4 5 0 0

update_hostname 8 1
run_and_append2 1200 8 1 0 0 2


update_hostname 8 8
run_and_append2 1200 8 8 0 0 2

update_hostname 4 4 4 4
run_and_append 1200 4 4

update_hostname 2 2 2 2 2 2 2
run_and_append 1200 8 2 0 0 2


update_hostname 8 8 8 1
run_and_append2 900 8 8 8 1 4

update_hostname 5 5 5 5 5
run_and_append 900 5 5

update_hostname 8 8 8 8 4
run_and_append2 900 8 8 8 8 4 5

# # Add total time column headers
# for run in {1..50}; do
#   echo -n "TotalTime${run},"
# done >> timings600.csv


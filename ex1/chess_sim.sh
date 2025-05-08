#!/bin/bash 

# Check that the file is exist
if [ ! -f "$1" ]; then
    echo "File does not exist: $1"
    exit 1
fi

echo "Metadata from PGN file:"
grep '^\[' "$1"
# Extract the moves
pgn_moves=$(grep -v '^\[' "$1" | tr '\n' ' ')

# Transferring the values
uci_moves=$(python3 parse_moves.py "$pgn_moves")

# Convert to array
read -a moves_history <<< "$uci_moves"

# Difine vraibles
current_step=0     
num_of_steps=${#moves_history[@]}


# Function to go through the next move
function nextmove() {
  if [ $current_step -ge $num_of_steps ]; then
    echo "No more moves available."
  else
    current_step=$((current_step + 1)) # increase current_step
    display_board
  fi
}


# Function to go back to the previous move
function previousmove(){
    if [ $current_step -le 0 ]; then 
        display_board
    else
        current_step=$((current_step - 1)) # decrease current_step
        display_board
    fi
}


# Function to jump to the beggining

function gotostart(){
    # update current_step
    current_step=0
    display_board
}


# Function to jmp to the last step
function gotoend(){
    # update current_step
    current_step=$num_of_steps
    display_board
}


#Function to quit from the game
function quit(){
    echo "Exiting."
    echo "End of game."
    exit 1
}



#print the board:
rows=8
columns=8



function display_board() {
    history="${moves_history[@]:0:$current_step}"
    board_output=$(python3 show_board.py "$history")
    echo "Move $current_step/$num_of_steps"
    echo "$board_output"
}

while true; do
  echo "Press 'd' to move forward, 'a' to move back, 'w' to go to the start, 's' to go to the end, 'q' to quit:"
  read -n 1 charecter
  echo

  case $charecter in
    d) nextmove ;;
    a) previousmove ;;
    w) gotostart ;;
    s) gotoend ;;
    q) quit ;;
    *) echo "Invalid key pressed: $charecter" ;;
  esac
done
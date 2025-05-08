#!/bin/bash 

# function that will show the args that input and return
function usage() {
    echo "Usage: $0 <source_pgn_file> <destination_directory>"
    exit 1
}

#check if the input is axactly two arguments
if [ "$#" -ne 2 ]; then
    usage
fi


# define the new arguments that we are going to work with
source_file="$1"
dest_dir="$2"
    
# check if the source file exist if not, end the run.
if [ ! -f "$source_file" ]; then
  echo "Error: File '$source_file' doest exist."
  exit 1
fi

# check if the source file exist. if not, the function will provide directory.
if [ ! -d "$dest_dir" ]; then
  mkdir -p "$dest_dir"
  echo "Created directory '$dest_dir'."
fi


# define arguments to store the data.
base_name="$(basename "$source_file" .pgn)"
# coute the games
game_count=0
# string that acqumulate the current game
currrent_game_lines="" 

# this loop will manege the data to current dir and divide it untill all the data processed.
while IFS= read -r line || [ -n "$line" ]; do

  if [[ $line =~ ^\[Event ]]; then
    if [ -n "$current_game_lines" ]; then
      ((game_count++))
      output_file="$dest_dir/${base_name}_${game_count}.pgn"
      echo "$current_game_lines" > "$output_file" # insert the data from the line to the file
      echo "Saved game to $output_file"
      current_game_lines="" # free the data that inserted.
    fi
  fi

  current_game_lines+="$line"$'\n'

done < "$source_file"

if [ -n "$current_game_lines" ]; then
  ((game_count++))
  output_file="$dest_dir/${base_name}_${game_count}.pgn"
  echo "$current_game_lines" > "$output_file"
  echo "Saved game to $output_file"
fi

# end messege
echo "All games have been split and saved to '$dest_dir'.
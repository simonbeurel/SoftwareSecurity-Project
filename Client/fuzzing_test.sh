#!/bin/bash

chemin_client="./client"

function fuzzing {
    regex="^sectrans -list$|^sectrans -up [a-zA-Z0-9_./]*$|^sectrans -down [a-zA-Z0-9_./]*$"
    while true
    do
        random_data=$(head -c 50 /dev/urandom | tr -dc 'a-zA-Z0-9_.')
        for input in "sectrans -list" "sectrans -up" "sectrans -down" "sectrans"
        do
            fuzz_input="$input $random_data"
            #echo "$fuzz_input"
            if [[ $fuzz_input =~ $regex ]]; then
                #$chemin_client "$fuzz_input"
                echo -e "admin\nadmin\n$fuzz_input\nexit\n" | $chemin_client
            else
                echo "Entrée non conforme à la regex: $fuzz_input"
            fi
        done
        sleep 1
    done
}
fuzzing



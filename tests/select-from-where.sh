#! /bin/bash

SQL="SELECT person.name FROM person Where name = 'joe';"

./systemr "$SQL"
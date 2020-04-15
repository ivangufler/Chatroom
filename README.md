# Chatroom

A chatroom project in C the programming language.
It implements a tcp client-server model, where several clients connect to one server.
Client and server use the poll-library to implement the model.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for testing purposes.

### Prerequisites

What things you need to install the software 

* GCC GNU Compiler
* GNU Make

and how to install them on Linux/OSX

```
sudo apt install gcc
sudo apt install make
```


### Installing

The repo contains all necessary files, you only have to compile the C-files and then run the executables.

If you want to get the executable file for the server, perform the following step:

```
make server
```

If you want to get the executable file for the client, perform the following step:

```
make client
```

If you want to get both the server and the client, perform the following step:

```
make
```

To run the server, perform:

```
./server.out
```

To run the client, perform:

```
./client.out [IPv4_OF_THE_SERVER]
```

If you want to recompile the project, use the following command before running the above commands again:

```
make clean
```

## Authors

* **Ivan Gufler** - [github/ivangufler](https://github.com/ivangufler)
* **Filippo Costanzi** - [github/costanzifilippo](https://github.com/costanzifilippo)

See also the list of [contributors](https://github.com/ivangufler/Chatroom/contributors) who participated in this project.

## License

This project is licensed under the GPL License - Feel free to use, study, change and share.

## Acknowledgments

* This project was created for a task assignment in the subject Systems and Networks.
* TFO "Max Valier" Bolzano/Bozen, 4th year


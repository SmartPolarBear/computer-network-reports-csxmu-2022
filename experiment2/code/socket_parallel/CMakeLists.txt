cmake_minimum_required(VERSION 3.22)

project(socket)

add_executable(server server.c)

add_executable(client client.c)
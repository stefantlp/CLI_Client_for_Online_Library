# CLI-Client-for-Online-Library

This repository contains a **C command-line client** for an online library, developed as part of the **Communication Protocols** course in the second year, second semester of my studies at university. The application interacts with an online library server via a **RESTful API** and uses **HTTP requests** for authentication, book management, and session handling.

## 📂 Repository Structure  

- **Source Files:**
  - [Buffer](src/buffer.c) – Handles buffer operations for HTTP requests and responses.  
  - [Buffer Header](src/buffer.h) – Header file for buffer-related functions.  
  - [Client](src/client.c) – The main entry point of the application, managing user input and command execution.  
  - [Helpers](src/helpers.c) – Utility functions for handling sockets and sending/receiving data.  
  - [Helpers Header](src/helpers.h) – Header file for helper functions.  
  - [Parson](src/parson.c) – JSON parser used for handling API responses and requests.  
  - [Parson Header](src/parson.h) – Header file for the Parson JSON parser.  
  - [Requests](src/requests.c) – Constructs and sends HTTP requests to the server.  
  - [Requests Header](src/requests.h) – Header file for request handling functions.  
  - [Makefile](src/Makefile) – Build script to compile the application.  

## 🛠 Features  

- **User Registration**: Create a new account by providing a username and password.  
- **User Login**: Authenticate with the server and receive a session cookie.  
- **Access Library**: Request access to the online library using an authentication token.  
- **View Available Books**: Retrieve and display a list of books from the library.  
- **View Book Details**: Search for a book by ID and display its details.  
- **Add a Book**: Add a new book to the library by providing title, author, genre, publisher, and page count.  
- **Delete a Book**: Remove a book from the library using its ID.  
- **Logout**: End the current session by logging out from the system.  

## 🖥️ Technologies Used  

- **C++** – Main programming language used.  
- **Sockets (BSD Sockets API)** – Used for network communication.  
- **HTTP Protocol** – Used to send requests and receive responses from the online library server.  
- **JSON (Parson Library)** – Used for handling API responses and requests.  
- **Makefile** – Used for compiling the project efficiently.  

## 🚀 Running the Application  

To build and run the application, simply execute these two commands: `make`, `./client`.

#include <dpp/dpp.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


void send_message_from_fifo(dpp::cluster& bot, dpp::snowflake channel_id) {
    const char* fifoPath = "/home/raspi1/Documents/POCSAG_bot/montcoFifo";
    char buffer[1200];

    while (true) {
        // Open the FIFO for reading
        int fd = open(fifoPath, O_RDONLY);
        if (fd < 0) {
            std::cerr << "Failed to open FIFO for reading: " << strerror(errno) << std::endl;
        }

        // Read data from the FIFO
        ssize_t bytesRead = read(fd, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            std::cout << "Received: " << buffer << std::endl;
            // Create the message
            dpp::message msg;
            msg.content = buffer;
            msg.channel_id = channel_id;

            // Send the message
            bot.message_create(msg, [](const dpp::confirmation_callback_t& callback) {
                if (callback.is_error()) {
                    std::cerr << "Failed to send message";
                }
            });
        }   
        close(fd);
    }
}

int main() {
    dpp::cluster bot(BOT_TOKEN);

    bot.on_log(dpp::utility::cout_logger());

    bot.on_ready([&bot](const dpp::ready_t& event) {
        std::cout << "Bot is online!" << std::endl;

        // Start the FIFO  message sending in a separate thread
        std::thread messageThread(send_message_from_fifo, std::ref(bot), CHANNEL_ID);
        messageThread.detach(); // Detach the thread to allow it to run independently
    });

    bot.start(dpp::st_wait);
    return 0;
}
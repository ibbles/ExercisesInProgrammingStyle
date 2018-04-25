#include <cstring>
#include <fstream>
#include <iostream>

/*
Good Old Times

- Very small amount of memory, much less than the size of the data.
- No identifiers, only tagged memory addresses.
*/



namespace memory {
    char bytes[4096];

    // [0-3] Index of line start.
    // Gives the address in memory where line data starts.
    // The index of line start will always be 13.
    const int LINE_START{0};
    // [4-7] Index of char_start of word.
    // Gives the address in memory where the current word in the line starts.
    const int WORD_START{4};
    // [8-11] Index on characters.
    // Don't know what this is.
    const int CHAR_INDEX{8};
    // [12] Flag indicating if word was found.
    const int FOUND_WORD{12};
    // [13-93] The line. Max 80 char, including newline.
    // [93-??] Word,NNNN.  Don't know what this is.
    // [??-??} Frequency.  Don't know what this is. Guessing an in.


    template<typename T>
    T
    read(int address) {
        T v;
        memcpy(&v, bytes + address, sizeof(v));
        return v;
    }


    template<typename T>
    void
    write(T v, int address) {
        memcpy(bytes + address, &v, sizeof(v));
    }
}


namespace disk {
    char bytes[1048576];

    // [0-3] Start of stop_words.
    const int STOP_WORDS_START{0};
    // [4-7] Size of stop_words.
    const int STOP_WORDS_SIZE{4};
    // [8-11] Start of text.
    const int TEXT_START{8};
    // [12-15] Size of text.
    const int TEXT_SIZE{12};
    // [16-19] Current text offset.
    const int TEXT_POS{16};
    // [20-23] Start of word_freq.
    const int WORD_FREQ_START{20};

    // [24-??] Stop words.
    // [??-??] Text.
    // [??-end] Word_freq.

    template<typename T>
    void
    write(T v, int offset) {
        memcpy(bytes + offset, &v, sizeof(v));
    }


    template<typename T>
    T
    read(int offset) {
        T v;
        memcpy(&v, bytes + offset, sizeof(v));
        return v;
    }
}


void
boot_stop_words() {
    disk::write(24, disk::STOP_WORDS_START);
    std::ifstream stop_words("stop_words.txt", std::ios_base::binary);
    stop_words.read(disk::bytes + disk::read<int>(disk::STOP_WORDS_START), sizeof(disk::bytes) - 20);
    disk::write(static_cast<int>(stop_words.gcount()), disk::STOP_WORDS_SIZE);
}


void
boot_text() {
    int start = disk::read<int>(disk::STOP_WORDS_START) + disk::read<int>(disk::STOP_WORDS_SIZE);
    std::ifstream text("pride-and-prejudice.txt", std::ios_base::binary);
    text.read(disk::bytes + start, sizeof(disk::bytes) - start);
    disk::write(start, disk::TEXT_START);
    disk::write(static_cast<int>(text.gcount()), disk::TEXT_SIZE);
    disk::write(disk::read<int>(disk::TEXT_START) + disk::read<int>(disk::TEXT_SIZE), disk::WORD_FREQ_START);
    disk::write(0, disk::TEXT_POS);
}


void
boot() {
    boot_stop_words();
    boot_text();

    memory::write(13, memory::LINE_START);
    memory::write(13, memory::WORD_START);

    std::cout << "Start of stop words: " << disk::read<int>(disk::STOP_WORDS_START) << '\n';
    std::cout << "Size of stop worlds: " << disk::read<int>(disk::STOP_WORDS_SIZE) << '\n';
    std::cout << "Start of text: " << disk::read<int>(disk::TEXT_START) << '\n';
    std::cout << "Size of text: " << disk::read<int>(disk::TEXT_SIZE) << '\n';
    std::cout << "Start of word_freq: " << disk::read<int>(disk::WORD_FREQ_START) << '\n';
    std::cout << "A few words: ";
    std::cout.write(disk::bytes + disk::read<int>(disk::STOP_WORDS_START), 50) << '\n';
    std::cout << "A bit of text: ";
    std::cout.write(disk::bytes + disk::read<int>(disk::TEXT_START), 50) << '\n';
}


void
read_line() {
    int n{0};
    char c = 0;
    while (n < disk::read<int>(disk::TEXT_SIZE) && c != '\n') {
        c = disk::read<char>(disk::read<int>(disk::TEXT_START) + disk::read<int>(disk::TEXT_POS));
        if (c != '\n') {
            memory::write(c, memory::read<int>(memory::LINE_START) + n);
            ++n;
        }
        disk::write(disk::read<int>(disk::TEXT_POS) + 1, disk::TEXT_POS);
    }

    memory::write(0, memory::read<int>(memory::LINE_START) + n);
}


int
main() {
    boot();

    int num_lines{0};

    while (true) {
        read_line();
        if (memory::read<char>(memory::read<int>(memory::LINE_START)) == 0) {
            break;
        }
        ++num_lines;
    }

    std::cout << "Processed " << num_lines << " lines.\n";
}

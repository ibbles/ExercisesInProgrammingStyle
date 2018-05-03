#include <cstring>
#include <fstream>
#include <iostream>

/*
Good Old Times
- Very small amount of memory, much less than the size of the data.
- No identifiers, only tagged memory addresses.

I did not finish this one. The Python implementation presented in the book takes
several shortcuts with regards to the memory restrictions, and I simply worked
around the no identifiers constraint by naming constraints and using getter
functions. A true no identifiers implementation is more work than I'm willing to
put into this. Perhaps I'll get back to this later.

When reading the chapter title and the constraints description I expected a more
assembly-like linear memory implementation, which is what I started out
implementing, but the Python code is not that.

Despite the criticism of the example code, I fully agree with the sentiment of
the chapter. Perhaps more so with regards to the limited amount of memory rather
than the lack of identifiers, which is just masochistic. It seems that as
computing power and available memory increase, there is always some use case
creeping up from below, finding utility in cheap, small, or otherwise
constrained general purpose devices. For a while it was video game consoles,
then smart phones, and now we have fully-fledged computers in our wrist watches
and classes. Limited memory devices that rely on secondary storage not from a
hard drive or SSD, but via some kind of network connection to a phone, nearby
computer or even a cloude based platform. The example the book uses, that of big
data processing, is also a good example of cases where the available memory,
despite being vast, is indeed limited.

I think the no identifiers requirement is mainly about making the reader consider
and reflect on the value of good names for a while. This is one of the hard
problems in computing science and on that I don't expect to be solved in general
any time soon.
*/



namespace memory {
    char bytes[4096];

    // [0-3] Index of line start.
    // Gives the address in memory where line data starts.
    // The index of line start will always be 13.
    const int LINE_START{0};
    // [4-7] Index of start_char of word.
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

extern bool todo_bool();

int
main() {
    boot();

    int num_lines{0};

    while (true) {
        read_line();
        if (memory::read<char>(memory::read<int>(memory::LINE_START)) == 0) {
            // End of input file.
            break;
        }
        memory::write(-1, memory::WORD_START);
        memory::write(0, memory::CHAR_INDEX);
        char c = 1; // Just anything non-zero.

        // Loop over characters in the line. Line start is pointed to by LINE_START,
        // and the line ends with a terminating 0.
        for (auto i = memory::read<int>(memory::LINE_START); c != 0; i++) {
            c = memory::read<char>(i);
            if (memory::read(memory::WORD_START) == -1) {
                if (isalnum(c)) {
                    // We found the start of a word.
                    memory::write(memory::read<int>(memory::CHAR_INDEX), memory::WORD_START);
                }
            } else {
                if (!isalnum(c)) {
                    // We found the end of a word. Process it.
                    // \todo Code here.
                    if (todo_bool() /* Word is long && Word not in stop words. */) {
                        // Lets see if it already exists.
                        while (true) {
                            /// \todo Read a line from word_freqs.
                            if (todo_bool() /* No more lines in word_freqs*/) {
                                break;
                            }
                            /// \todo Read freq from word_freq line.
                            /// \todo Read word from word_freq line.
                            if (true /* word == word */) {
                                /// \todo Increment freq.
                                /// \todo Set FOUND_WORD flag.
                                memory::write(1, memory::FOUND_WORD);
                                break;
                            }
                        }

                        if (!memory::read<int>(memory::FOUND_WORD)) {
                            /// \todo Write a new line to word_freq with word and 1.
                        } else {
                            /// \todo Overwrite word_freq line with word and incremented freq.
                            /// \todo Each line formatted so that the word field
                            ///       is 20 characters wide and the freq 4 wide.
                            ///       This makes the entire line 26 characters.
                            ///       20 + 1 + 4 + 1 where the two ones are ','
                            ///       and '\n'.
                        }
                        /// \todo Seek back to beginning of word_freq for next iteration.
                    }
                    // Let's reset.
                    /// \todo Set start_char to 0.
                }
                /// \todo Increment CHAR_INDEX.
            }
        }
        ++num_lines;
    }

    std::cout << "Processed " << num_lines << " lines.\n";
}

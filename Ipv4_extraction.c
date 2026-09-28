// Name = Joshua Fakunmoju
// Class = Software Engineering II
// Detail = Extracting IPv4 Addresses from Noisy Text
// Source(s) = Gemini


#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Helper function to parse an integer from str starting at *index up to maxValue.
static int parseInteger(const char* str, int* index, unsigned int maxValue, unsigned int* outValue) {
    int curr = *index; // Track the current position in the input string

    // Return failure if current character is not a digit
    if (!isdigit((unsigned char)str[curr])) {
        return 0;
    }

    // Reject leading zeros unless the digit itself is a standalone zero '0'
    if (str[curr] == '0' && isdigit((unsigned char)str[curr + 1])) {
        return 0;
    }

    unsigned long val = 0;  // Accumulator for the parsed numeric value
    int digitCount = 0;     // Counter for number of digits parsed

    // Read digits sequentially and accumulate value
    while (isdigit((unsigned char)str[curr])) {
        val = val * 10 + (str[curr] - '0'); // Convert char digit to numerical value
        digitCount++;                        // Increment digit counter
        curr++;                              // Advance character reader pointer

        // Reject immediately if the accumulated value exceeds allowed threshold
        if (val > maxValue) {
            return 0;
        }
    }

    // Safety check to ensure at least one valid digit was read
    if (digitCount == 0) {
        return 0;
    }

    *outValue = (unsigned int)val; // Write parsed integer value to output location
    *index = curr;                 // Update caller's index pointer to new position
    return 1;                      // Return success signal
}

// Main extraction function: scans str for a single valid IPv4 address and optional port
int extractIPv4(const char* str, unsigned long* outAddress, int* outPort) {
    *outAddress = 0;  // Initialize output address to default zero
    *outPort = -1;     // Initialize output port to -1 (indicates no port present)

    int len = (int)strlen(str); // Calculate length of input string

    // Outer loop: iterate through each character in the string as a potential starting candidate
    for (int i = 0; i < len; i++) {
        // Candidate start must begin on a digit
        if (!isdigit((unsigned char)str[i])) {
            continue; // Skip non-digit starting characters
        }

        // Left boundary check: candidate cannot be directly preceded by a digit, dot, or colon
        if (i > 0 && (isdigit((unsigned char)str[i - 1]) || str[i - 1] == '.' || str[i - 1] == ':')) {
            continue; // Skip candidate if attached to invalid left boundary token
        }

        int curr = i;           // Local reading offset starting at current candidate index
        unsigned int octets[4]; // Array to hold the 4 parsed IP octets
        int valid = 1;          // Flag tracking candidate validity

        // Parse 4 octets separated by '.'
        for (int k = 0; k < 4; k++) {
            // Require a dot before octets 2, 3, and 4
            if (k > 0) {
                if (str[curr] == '.') {
                    curr++; // Skip dot separator
                } else {
                    valid = 0; // Mark invalid if expected dot separator is missing
                    break;
                }
            }

            // Parse individual octet (max value 255)
            if (!parseInteger(str, &curr, 255, &octets[k])) {
                valid = 0; // Mark invalid if integer parsing fails or exceeds 255
                break;
            }
        }

        // Skip to next starting position if IPv4 octet parsing failed
        if (!valid) {
            continue;
        }

        int parsedPort = -1; // Temporary storage for optional port
        // Check for optional port delimiter ':'
        if (str[curr] == ':') {
            curr++; // Skip ':'
            
            // Require immediate digit following colon
            if (!isdigit((unsigned char)str[curr])) {
                valid = 0; // Mark invalid if colon is not followed by a digit
            } else {
                unsigned int tempPort = 0; // Temporary storage for port value
                
                // Parse port integer (max value 65535)
                if (parseInteger(str, &curr, 65535, &tempPort)) {
                    // Reject port truncation inside alphanumeric token (e.g., :8080abc)
                    if (isalnum((unsigned char)str[curr])) {
                        valid = 0; // Reject if port is attached to alphanumeric characters
                    } else {
                        parsedPort = (int)tempPort; // Store parsed port value
                    }
                } else {
                    valid = 0; // Mark invalid if port parsing fails or exceeds 65535
                }
            }
        }

        // Skip to next starting position if port validation failed
        if (!valid) {
            continue;
        }

        // Right boundary check: candidate cannot be immediately followed by '.', ':', or a digit
        if (str[curr] == '.' || str[curr] == ':' || isdigit((unsigned char)str[curr])) {
            continue; // Skip candidate if attached to invalid trailing token characters
        }

        // Construct 32-bit integer representation from octets using bitwise shifts
        *outAddress = ((unsigned long)octets[0] << 24) |
                      ((unsigned long)octets[1] << 16) |
                      ((unsigned long)octets[2] << 8)  |
                       (unsigned long)octets[3];
        *outPort = parsedPort; // Store final parsed port value (-1 if no port specified)
        return 1;              // Return success signal on valid extraction
    }

    return 0; // Return 0 if no valid IPv4 address was found in input string
}

// Driver main routine to demonstrate function execution
int main(void) {
    char input[1024]; // Buffer for console input string

    // Infinite loop processing input until user enters 'END'
    while (1) {
        printf("Enter a string (or 'END' to quit): ");
        // Read line from stdin safely
        if (!fgets(input, sizeof(input), stdin)) {
            break; // Break loop on EOF or read failure
        }

        size_t len = strlen(input); // Compute length of line read
        // Strip newline character if present
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        // Check for exit condition
        if (strcmp(input, "END") == 0) {
            printf("Program terminated.\n");
            break; // Exit loop
        }

        unsigned long outAddress = 0; // Variable to hold extracted address integer
        int outPort = -1;             // Variable to hold extracted port integer

        // Perform extraction attempt
        if (extractIPv4(input, &outAddress, &outPort)) {
            // Unpack 32-bit address integer back into 4 discrete octet bytes for display
            unsigned int a = (outAddress >> 24) & 0xFF;
            unsigned int b = (outAddress >> 16) & 0xFF;
            unsigned int c = (outAddress >> 8)  & 0xFF;
            unsigned int d = outAddress & 0xFF;

            // Display extracted output with or without port
            if (outPort != -1) {
                printf("Extracted IPv4 address: %u.%u.%u.%u (decimal value: %lu, port: %d)\n",
                       a, b, c, d, outAddress, outPort);
            } else {
                printf("Extracted IPv4 address: %u.%u.%u.%u (decimal value: %lu, port: none)\n",
                       a, b, c, d, outAddress);
            }
        } else {
            printf("Invalid input: no valid IPv4 address found\n"); // Print failure notice
        }
    }

    return 0; // Return zero status code on normal termination
}
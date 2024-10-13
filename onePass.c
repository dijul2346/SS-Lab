#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define structure for the symbol table (SYMTAB)
typedef struct ForwardRef {
    int loc; // location waiting to be resolved
    struct ForwardRef* next;
} ForwardRef;

typedef struct Symbol {
    char label[10];
    int address;
    ForwardRef* forwardList;
    struct Symbol* next;
} Symbol;

Symbol* symtab = NULL;

// Define memory for the object code
#define MEMORY_SIZE 1000
char memory[MEMORY_SIZE];

// Function to create a new symbol table entry
Symbol* createSymbol(char* label, int address) {
    Symbol* newSymbol = (Symbol*)malloc(sizeof(Symbol));
    strcpy(newSymbol->label, label);
    newSymbol->address = address;
    newSymbol->forwardList = NULL;
    newSymbol->next = NULL;
    return newSymbol;
}

// Function to insert a symbol into the SYMTAB
void insertSymbol(char* label, int address) {
    Symbol* sym = createSymbol(label, address);
    sym->next = symtab;
    symtab = sym;
}

// Function to find a symbol in the SYMTAB
Symbol* findSymbol(char* label) {
    Symbol* temp = symtab;
    while (temp != NULL) {
        if (strcmp(temp->label, label) == 0) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

// Function to add a forward reference
void addForwardRef(Symbol* sym, int loc) {
    ForwardRef* ref = (ForwardRef*)malloc(sizeof(ForwardRef));
    ref->loc = loc;
    ref->next = sym->forwardList;
    sym->forwardList = ref;
}

// Function to resolve forward references for a symbol
void resolveForwardRefs(Symbol* sym) {
    ForwardRef* ref = sym->forwardList;
    while (ref != NULL) {
        memory[ref->loc] = sym->address; // Update memory with resolved address
        ref = ref->next;
    }
}

// Function to simulate opcode table lookup
int getOpcode(char* opcode) {
    if (strcmp(opcode, "LDA") == 0) return 0x00;
    if (strcmp(opcode, "STA") == 0) return 0x0C;
    if (strcmp(opcode, "LDX") == 0) return 0x04;
    return -1; // Invalid opcode
}

// Function to handle the Load and Go algorithm
void loadAndGo() {
    int LOCCTR = 0;
    char line[100], label[10], opcode[10], operand[10];

    // Simulate reading input lines (in actual case, this would be from a file)
    while (fgets(line, sizeof(line), stdin)) {
        sscanf(line, "%s %s %s", label, opcode, operand);

        // Check if the opcode is START
        if (strcmp(opcode, "START") == 0) {
            LOCCTR = atoi(operand); // Set starting address
            continue;
        }

        // Search for the label in SYMTAB and insert it if not present
        if (label[0] != '\0') {
            Symbol* sym = findSymbol(label);
            if (sym == NULL) {
                insertSymbol(label, LOCCTR);
            } else if (sym->address == -1) {
                sym->address = LOCCTR;
                resolveForwardRefs(sym); // Resolve any forward references
            }
        }

        // Opcode processing
        int objCode = getOpcode(opcode);
        if (objCode != -1) {
            memory[LOCCTR++] = objCode; // Load opcode into memory

            // Search for operand in SYMTAB
            Symbol* sym = findSymbol(operand);
            if (sym != NULL) {
                if (sym->address != -1) {
                    memory[LOCCTR++] = sym->address; // Load operand address
                } else {
                    addForwardRef(sym, LOCCTR++);
                }
            } else {
                insertSymbol(operand, -1); // Insert with undefined address
                addForwardRef(findSymbol(operand), LOCCTR++);
            }
        }
        // Handle directives like WORD, RESW, RESB, BYTE
        else if (strcmp(opcode, "WORD") == 0) {
            int value = atoi(operand);
            memory[LOCCTR++] = value;
        }
    }
}

// Main function
int main() {
    loadAndGo();

    // Display the memory (hexadecimal code)
    printf("Memory Contents:\n");
    for (int i = 0; i < MEMORY_SIZE && memory[i] != 0; i++) {
        printf("%02X ", memory[i] & 0xFF);
        if ((i + 1) % 16 == 0) printf("\n");
    }

    return 0;
}

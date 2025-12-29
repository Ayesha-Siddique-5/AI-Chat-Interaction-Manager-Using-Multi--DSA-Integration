#include <iostream>
using namespace std;

// ============================================
// HELPER FUNCTIONS
// ============================================

void copyString(char* dest, const char* src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

int stringLength(const char* str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

void getCurrentTime(char* buffer) {
    // Simple timestamp - just use message count or fixed format
    copyString(buffer, "2024-12-25 10:30:00");
}

// ============================================
// DOUBLY LINKED LIST - Message Storage
// ============================================

struct Message {
    int id;
    char sender[10];        // "User" or "AI"
    char timestamp[30];
    char text[500];
    int replyTo;           // -1 if not a reply
    Message* prev;
    Message* next;
};

struct ChatHistory {
    Message* head;
    Message* tail;
    int messageCount;
};

void initChatHistory(ChatHistory* chat) {
    chat->head = NULL;
    chat->tail = NULL;
    chat->messageCount = 0;
}

void addMessage(ChatHistory* chat, const char* sender, const char* text, int replyTo = -1) {
    Message* newMsg = new Message;
    newMsg->id = chat->messageCount + 1;
    copyString(newMsg->sender, sender);
    getCurrentTime(newMsg->timestamp);
    copyString(newMsg->text, text);
    newMsg->replyTo = replyTo;
    newMsg->next = NULL;
    newMsg->prev = chat->tail;
    
    if (chat->tail == NULL) {
        chat->head = newMsg;
        chat->tail = newMsg;
    } else {
        chat->tail->next = newMsg;
        chat->tail = newMsg;
    }
    
    chat->messageCount++;
    cout << "\n[Message Added] ID: " << newMsg->id << " | " << sender << ": " << text << endl;
}

void displayAllMessages(ChatHistory* chat) {
    cout << "\n========== CHAT HISTORY ==========\n";
    Message* current = chat->head;
    while (current != NULL) {
        cout << "ID: " << current->id << " | " << current->sender << endl;
        cout << "Time: " << current->timestamp << endl;
        cout << "Message: " << current->text << endl;
        if (current->replyTo != -1) {
            cout << "(Reply to Message ID: " << current->replyTo << ")" << endl;
        }
        cout << "-----------------------------------\n";
        current = current->next;
    }
}

Message* findMessageById(ChatHistory* chat, int id) {
    Message* current = chat->head;
    while (current != NULL) {
        if (current->id == id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// ============================================
// STACKS - Undo and Redo Operations
// ============================================

struct StackNode {
    int messageId;
    char originalText[500];
    StackNode* next;
};

struct Stack {
    StackNode* top;
    int size;
};

void initStack(Stack* stack) {
    stack->top = NULL;
    stack->size = 0;
}

void push(Stack* stack, int messageId, const char* text) {
    StackNode* newNode = new StackNode;
    newNode->messageId = messageId;
    copyString(newNode->originalText, text);
    newNode->next = stack->top;
    stack->top = newNode;
    stack->size++;
}

bool pop(Stack* stack, int* messageId, char* text) {
    if (stack->top == NULL) {
        return false;
    }
    
    StackNode* temp = stack->top;
    *messageId = temp->messageId;
    copyString(text, temp->originalText);
    stack->top = stack->top->next;
    delete temp;
    stack->size--;
    return true;
}

bool isEmpty(Stack* stack) {
    return stack->top == NULL;
}

void editMessage(ChatHistory* chat, Stack* undoStack, Stack* redoStack, int messageId, const char* newText) {
    Message* msg = findMessageById(chat, messageId);
    if (msg == NULL) {
        cout << "\n[Error] Message ID " << messageId << " not found!\n";
        return;
    }
    
    // Save original text to undo stack
    push(undoStack, messageId, msg->text);
    
    // Clear redo stack on new edit
    while (!isEmpty(redoStack)) {
        int dummy;
        char dummyText[500];
        pop(redoStack, &dummy, dummyText);
    }
    
    // Update message
    copyString(msg->text, newText);
    cout << "\n[Message Edited] ID: " << messageId << " | New Text: " << newText << endl;
}

void undoEdit(ChatHistory* chat, Stack* undoStack, Stack* redoStack) {
    if (isEmpty(undoStack)) {
        cout << "\n[Info] Nothing to undo!\n";
        return;
    }
    
    int messageId;
    char originalText[500];
    pop(undoStack, &messageId, originalText);
    
    Message* msg = findMessageById(chat, messageId);
    if (msg != NULL) {
        // Save current text to redo stack
        push(redoStack, messageId, msg->text);
        
        // Restore original text
        copyString(msg->text, originalText);
        cout << "\n[Undo] Message ID " << messageId << " restored to: " << originalText << endl;
    }
}

void redoEdit(ChatHistory* chat, Stack* undoStack, Stack* redoStack) {
    if (isEmpty(redoStack)) {
        cout << "\n[Info] Nothing to redo!\n";
        return;
    }
    
    int messageId;
    char redoText[500];
    pop(redoStack, &messageId, redoText);
    
    Message* msg = findMessageById(chat, messageId);
    if (msg != NULL) {
        // Save current text to undo stack
        push(undoStack, messageId, msg->text);
        
        // Apply redo text
        copyString(msg->text, redoText);
        cout << "\n[Redo] Message ID " << messageId << " changed to: " << redoText << endl;
    }
}

// ============================================
// QUEUE - Important Messages
// ============================================

struct QueueNode {
    int messageId;
    QueueNode* next;
};

struct Queue {
    QueueNode* front;
    QueueNode* rear;
    int size;
};

void initQueue(Queue* queue) {
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
}

void enqueue(Queue* queue, int messageId) {
    QueueNode* newNode = new QueueNode;
    newNode->messageId = messageId;
    newNode->next = NULL;
    
    if (queue->rear == NULL) {
        queue->front = newNode;
        queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
    queue->size++;
}

void markImportant(ChatHistory* chat, Queue* importantQueue, int messageId) {
    Message* msg = findMessageById(chat, messageId);
    if (msg == NULL) {
        cout << "\n[Error] Message ID " << messageId << " not found!\n";
        return;
    }
    
    enqueue(importantQueue, messageId);
    cout << "\n[Marked Important] Message ID: " << messageId << endl;
}

void displayImportantMessages(ChatHistory* chat, Queue* importantQueue) {
    cout << "\n========== IMPORTANT MESSAGES ==========\n";
    if (importantQueue->front == NULL) {
        cout << "No important messages marked.\n";
        return;
    }
    
    QueueNode* current = importantQueue->front;
    while (current != NULL) {
        Message* msg = findMessageById(chat, current->messageId);
        if (msg != NULL) {
            cout << "ID: " << msg->id << " | " << msg->sender << ": " << msg->text << endl;
        }
        current = current->next;
    }
    cout << "========================================\n";
}

// ============================================
// GRAPH - Reply and Thread Structure
// ============================================

struct GraphNode {
    int messageId;
    GraphNode* next;
};

struct Graph {
    GraphNode** adjList;
    int maxMessages;
};

void initGraph(Graph* graph, int maxMessages) {
    graph->maxMessages = maxMessages;
    graph->adjList = new GraphNode*[maxMessages + 1];
    for (int i = 0; i <= maxMessages; i++) {
        graph->adjList[i] = NULL;
    }
}

void addReplyEdge(Graph* graph, int fromMessageId, int toMessageId) {
    GraphNode* newNode = new GraphNode;
    newNode->messageId = toMessageId;
    newNode->next = graph->adjList[fromMessageId];
    graph->adjList[fromMessageId] = newNode;
}

void displayThreadStructure(Graph* graph, ChatHistory* chat) {
    cout << "\n========== REPLY THREAD STRUCTURE ==========\n";
    for (int i = 1; i <= chat->messageCount; i++) {
        if (graph->adjList[i] != NULL) {
            cout << "Message ID " << i << " has replies:\n";
            GraphNode* current = graph->adjList[i];
            while (current != NULL) {
                Message* msg = findMessageById(chat, current->messageId);
                if (msg != NULL) {
                    cout << "  -> ID " << current->messageId << ": " << msg->text << endl;
                }
                current = current->next;
            }
        }
    }
    cout << "============================================\n";
}

// ============================================
// MAIN PROGRAM
// ============================================

void displayMenu() {
    cout << "\n========== AI CHAT INTERACTION MANAGER ==========\n";
    cout << "1.  Add User Message\n";
    cout << "2.  Add AI Message\n";
    cout << "3.  Add Reply to a Message\n";
    cout << "4.  Display All Messages\n";
    cout << "5.  Edit Message\n";
    cout << "6.  Undo Last Edit\n";
    cout << "7.  Redo Last Edit\n";
    cout << "8.  Mark Message as Important\n";
    cout << "9.  Display Important Messages\n";
    cout << "10. Display Reply Thread Structure\n";
    cout << "0.  Exit\n";
    cout << "=================================================\n";
    cout << "Enter your choice: ";
}

int main() {
    ChatHistory chat;
    initChatHistory(&chat);
    
    Stack undoStack, redoStack;
    initStack(&undoStack);
    initStack(&redoStack);
    
    Queue importantQueue;
    initQueue(&importantQueue);
    
    Graph replyGraph;
    initGraph(&replyGraph, 100); // Support up to 100 messages
    
    int choice;
    char text[500];
    int messageId, replyToId;
    
    cout << "\n*** AI CHAT INTERACTION MANAGER ***\n";
    cout << "Multi-DSA Integration Project\n";
    
    while (true) {
        displayMenu();
        cin >> choice;
        cin.ignore(); // Clear newline
        
        switch (choice) {
            case 1:
                cout << "Enter user message: ";
                cin.getline(text, 500);
                addMessage(&chat, "User", text);
                break;
                
            case 2:
                cout << "Enter AI message: ";
                cin.getline(text, 500);
                addMessage(&chat, "AI", text);
                break;
                
            case 3:
                cout << "Enter message ID to reply to: ";
                cin >> replyToId;
                cin.ignore();
                
                if (findMessageById(&chat, replyToId) == NULL) {
                    cout << "\n[Error] Message ID " << replyToId << " not found!\n";
                    break;
                }
                
                cout << "Enter reply text: ";
                cin.getline(text, 500);
                
                cout << "Is this reply from (1) User or (2) AI? ";
                int senderChoice;
                cin >> senderChoice;
                cin.ignore();
                
                addMessage(&chat, (senderChoice == 1) ? "User" : "AI", text, replyToId);
                addReplyEdge(&replyGraph, replyToId, chat.messageCount);
                break;
                
            case 4:
                displayAllMessages(&chat);
                break;
                
            case 5:
                cout << "Enter message ID to edit: ";
                cin >> messageId;
                cin.ignore();
                cout << "Enter new text: ";
                cin.getline(text, 500);
                editMessage(&chat, &undoStack, &redoStack, messageId, text);
                break;
                
            case 6:
                undoEdit(&chat, &undoStack, &redoStack);
                break;
                
            case 7:
                redoEdit(&chat, &undoStack, &redoStack);
                break;
                
            case 8:
                cout << "Enter message ID to mark as important: ";
                cin >> messageId;
                markImportant(&chat, &importantQueue, messageId);
                break;
                
            case 9:
                displayImportantMessages(&chat, &importantQueue);
                break;
                
            case 10:
                displayThreadStructure(&replyGraph, &chat);
                break;
                
            case 0:
                cout << "\n*** Exiting AI Chat Interaction Manager ***\n";
                cout << "Thank you for using the system!\n";
                return 0;
                
            default:
                cout << "\n[Error] Invalid choice! Please try again.\n";
        }
    }
    
    return 0;
}
#include <Windows.h>
#include <stdio.h>
#include <time.h>
#include <string>
#include <vector>
using namespace std;

#define READER_COUNT 4
#define WRITER_COUNT 2
#define DATA_SIZE 5
#define BUFFER_SIZE 20

struct Data {
	char* buffer;
	int threadNum;
};

HANDLE dataAccessMutex;
HANDLE rcMutex;
HANDLE consoleMutex;
int activeReaderCount = 0;

VOID readerThread(LPVOID param);
VOID writerThread(LPVOID param);

int main(int argc, char* argv[]) {
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	HANDLE hReaders[READER_COUNT];
	HANDLE hWriters[WRITER_COUNT];
	char buffer[BUFFER_SIZE];
	Data readersData[READER_COUNT];
	Data writersData[WRITER_COUNT];
	
	for (int i = 0; i < BUFFER_SIZE; i++)
		buffer[i] = ' ';

	dataAccessMutex = CreateMutexA(NULL, FALSE, "dataAccessMutex");
	rcMutex = CreateMutexA(NULL, FALSE, "rcMutex");
	consoleMutex = CreateMutexA(NULL, FALSE, "consoleMutex");

	for (int i = 0; i < WRITER_COUNT; i++) {
		writersData[i].buffer = buffer;
		writersData[i].threadNum = i + 1;

		hWriters[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) writerThread, &writersData[i], 0, 0);
	}

	for (int i = 0; i < READER_COUNT; i++) {
		readersData[i].buffer = buffer;
		readersData[i].threadNum = i + 1;

		hReaders[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)readerThread, &readersData[i], 0, 0);
	}

	WaitForMultipleObjects(WRITER_COUNT, hWriters, TRUE, INFINITE);
	WaitForMultipleObjects(READER_COUNT, hReaders, TRUE, INFINITE);

	system("pause");
	return 0;
}

VOID readerThread(LPVOID param) {
	Data* data = (Data*)param;

	WaitForSingleObject(rcMutex, INFINITE);
	if (++activeReaderCount == 1)
		WaitForSingleObject(dataAccessMutex, INFINITE);
	ReleaseMutex(rcMutex);

	string logRecord = "Читатель №" + to_string(data->threadNum) + ": ";
	for (int i = 0; i < BUFFER_SIZE; i++)
		logRecord.push_back(data->buffer[i]);
	logRecord.push_back('\n');

	WaitForSingleObject(consoleMutex, INFINITE);
	printf_s(logRecord.c_str());
	ReleaseMutex(consoleMutex);

	WaitForSingleObject(rcMutex, INFINITE);
	if (--activeReaderCount == 0)
		ReleaseMutex(dataAccessMutex);
	ReleaseMutex(rcMutex);
}

VOID writerThread(LPVOID param) {
	Data* data = (Data*)param;
	srand((unsigned int)time(NULL));

	WaitForSingleObject(dataAccessMutex, INFINITE);

	int position = rand() % BUFFER_SIZE;

	WaitForSingleObject(consoleMutex, INFINITE);
	printf_s("Писатель №%d\n", data->threadNum);
	ReleaseMutex(consoleMutex);

	for (int i = 0; i < DATA_SIZE; i++) {
		data->buffer[position] = (char)(rand() % 26 + 65);

		if (++position == BUFFER_SIZE)
			position = 0;
	}

	ReleaseMutex(dataAccessMutex);
}
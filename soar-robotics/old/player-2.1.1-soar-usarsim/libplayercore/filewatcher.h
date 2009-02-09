/*
 * filewatcher.h
 *
 *  Created on: 10/06/2008
 *      Author: tcollett
 */

#ifndef FILEWATCHER_H_
#define FILEWATCHER_H_

#include <sys/select.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pthread.h>
#include <libplayercore/message.h>

class MessageQueue;

struct fd_driver_pair
{
	int fd;
	QueuePointer queue;
	bool Read;
	bool Write;
	bool Except;
};

const size_t INITIAL_WATCHED_FILES_ARRAY_SIZE = 32;

class FileWatcher
{
public:
	FileWatcher();
	virtual ~FileWatcher();

	int Wait(double Timeout = 0);
	int AddFileWatch(int fd, QueuePointer & queue, bool WatchRead = true, bool WatchWrite = false, bool WatchExcept = true);
	int RemoveFileWatch(int fd, QueuePointer & queue, bool WatchRead = true, bool WatchWrite = false, bool WatchExcept = true);
	int AddFileWatch(int fd, bool WatchRead = true, bool WatchWrite = false, bool WatchExcept = true);
	int RemoveFileWatch(int fd, bool WatchRead = true, bool WatchWrite = false, bool WatchExcept = true);

private:
	struct fd_driver_pair * WatchedFiles;
	size_t WatchedFilesArraySize;
	size_t WatchedFilesArrayCount;

    /** @brief Lock access to watcher internals. */
    virtual void Lock(void);
    /** @brief Unlock access to watcher internals. */
    virtual void Unlock(void);
    /// Used to lock access to Data.
    pthread_mutex_t lock;

};

#endif /* FILEWATCHER_H_ */

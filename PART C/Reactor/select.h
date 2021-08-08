#ifndef __SELECT_H__
#define __SELECT_H__

int add_fd_to_monitoring(const unsigned int fd);      // Add fd to the reactor
int remove_fd_from_monitoring(const unsigned int fd); //Remove fd from monitoring

/**
 * This module was given in the class and it contains a Reactor design pattern
 */

int32_t wait_for_input();
extern int32_t alloced_fds_num; //How much FDs are currently monitored
extern int32_t *alloced_fds;    // array of the monitored FDs

#endif
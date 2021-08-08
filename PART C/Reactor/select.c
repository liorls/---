#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include "select.h"

#define TRUE (1)
#define FALSE (0)

static fd_set rfds, rfds_copy;
static int32_t max_fd = 0;
static int32_t initialized = FALSE;
int32_t *alloced_fds = NULL;
int32_t alloced_fds_num = 0;

static int32_t add_fd_to_monitoring_internal(const unsigned int fd)
{
  int32_t *tmp_alloc;
  tmp_alloc = realloc(alloced_fds, sizeof(int32_t) * (alloced_fds_num + 1));
  if (tmp_alloc == NULL)
    return -1;
  alloced_fds = tmp_alloc;
  alloced_fds[alloced_fds_num++] = fd;
  FD_SET(fd, &rfds_copy);
  if (max_fd < fd)
    max_fd = fd;

  return 0;
}

int rm_fd_internal(const uint32_t fd)
{
  int32_t *tmp_alloc;
  alloced_fds_num--;
  if (alloced_fds_num == 0)
  {
    free(alloced_fds);
    alloced_fds = NULL;
  }
  tmp_alloc = realloc(alloced_fds, sizeof(int32_t) * (alloced_fds_num));
  if (tmp_alloc == NULL)
    return -1;
  int i = 0;
  for (i = 0; i < alloced_fds_num; i++)
  {
    if (alloced_fds[i] != fd)
    {
      tmp_alloc[i] = alloced_fds[i];
    }
    else
    {
      break;
    }
  }
  //now fd = allocated[i]
  for (int j = i + i; j < alloced_fds_num; i++)
  {
    tmp_alloc[i++] = alloced_fds[j];
  }
  alloced_fds = tmp_alloc;
  return 0;
}

int32_t init()
{
  FD_ZERO(&rfds_copy);
  if (add_fd_to_monitoring_internal(0) < 0)
    return -1; // monitoring standard input
  initialized = TRUE;
  return 0;
}

int add_fd_to_monitoring(const unsigned int fd)
{
  if (!initialized)
    init();
  if (fd > 0)
    return add_fd_to_monitoring_internal(fd);
  return 0;
}
int remove_fd_from_monitoring(const unsigned int fd)
{
  if (!initialized)
    init();
  if (fd > 0)
    return rm_fd_internal(fd);
  return 0;
}

int32_t wait_for_input()
{
  int32_t i, retval;
  memcpy(&rfds, &rfds_copy, sizeof(rfds_copy));
  retval = select(max_fd + 1, &rfds, NULL, NULL, NULL);
  if (retval > 0)
  {
    for (i = 0; i < alloced_fds_num; ++i)
    {
      if (FD_ISSET(alloced_fds[i], &rfds))
        return alloced_fds[i];
    }
  }
  return -1;
}

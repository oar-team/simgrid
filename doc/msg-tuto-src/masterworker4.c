/* Copyright (c) 2007-2010, 2013-2015. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "simgrid/msg.h"

XBT_LOG_NEW_DEFAULT_CATEGORY(msg_test, "Messages specific for this msg example");

#define FINALIZE ((void*)221297)        /* a magic number to tell people to stop working */

static char * build_channel_name(char *buffer, const char *sender, const char* receiver)
{
  strcpy(buffer, sender);
  strcat(buffer, ":");
  strcat(buffer, receiver);
  return buffer;
}

/** Worker function  */
static int master(int argc, char *argv[])
{
  int workers_count = 0;
  XBT_ATTRIB_UNUSED int res;
  msg_host_t *workers = NULL;
  msg_task_t *todo = NULL;
  msg_host_t host_self = MSG_host_self();
  char *master_name = (char *) MSG_host_get_name(host_self);
  char channel[1024];

  int i;

  TRACE_category(master_name);

  double timeout   = xbt_str_parse_double(argv[1], "Invalid timeout: %s");             /** - timeout      */
  double comp_size = xbt_str_parse_double(argv[2], "Invalid computational size: %s");  /** - Task compute cost    */
  double comm_size = xbt_str_parse_double(argv[3], "Invalid communication size: %s");  /** - Task communication size */

  {                             /* Process organization */
    workers_count = MSG_get_host_number();
    workers = xbt_dynar_to_array(MSG_hosts_as_dynar());
    
    for (i = 0; i < workers_count; i++)
      if(host_self == workers[i]) {
        workers[i] = workers[workers_count-1];
        workers_count--;
        break;
      }

    for (i = 0; i < workers_count; i++)
      MSG_process_create("worker", worker, master_name, workers[i]);
  }

  XBT_INFO("Got %d workers and will send tasks for %g seconds!", workers_count, timeout);
  xbt_dynar_t idle_hosts = xbt_dynar_new(sizeof(msg_host_t), NULL);
  msg_host_t request_host = NULL;

  for (i = 0; 1;) {
    char sprintf_buffer[64];
    msg_task_t task = NULL;

    msg_task_t request = NULL;
    while(MSG_task_listen(master_name)) {
      res = MSG_task_receive(&(request),master_name);
      xbt_assert(res == MSG_OK, "MSG_task_receive failed");
      request_host = MSG_task_get_data(request);
      xbt_dynar_push(idle_hosts, &request_host);
      MSG_task_destroy(request);
      request = NULL;
    }

    if(MSG_get_clock()>timeout) {
      if(xbt_dynar_length(idle_hosts) == workers_count) break;
      else {
        MSG_process_sleep(.1);
        continue;
      }
    }

    if(xbt_dynar_length(idle_hosts)<=0) {
      /* No request. Let's wait... */
      MSG_process_sleep(.1);
      continue;
    }

    sprintf(sprintf_buffer, "Task_%d", i);
    task = MSG_task_create(sprintf_buffer, comp_size, comm_size, NULL);
    MSG_task_set_category(task, master_name);

    xbt_dynar_shift(idle_hosts, &request_host);

    build_channel_name(channel,master_name, MSG_host_get_name(request_host));

    XBT_DEBUG("Sending \"%s\" to channel \"%s\"", task->name, channel);
    MSG_task_send(task, channel);
    XBT_DEBUG("Sent");
    i++;
  }

  int task_num = i;

  XBT_DEBUG ("All tasks have been dispatched. Let's tell everybody the computation is over.");
  for (i = 0; i < workers_count; i++) {
    msg_task_t finalize = MSG_task_create("finalize", 0, 0, FINALIZE);
    MSG_task_send(finalize, build_channel_name(channel,master_name, MSG_host_get_name(workers[i % workers_count])));
  }

  XBT_INFO("Sent %d tasks in total!", task_num);
  free(workers);
  free(todo);
  return 0;
}                               /* end_of_master */

/** Worker function  */
static int worker(int argc, char *argv[])
{
  msg_task_t task = NULL;
  XBT_ATTRIB_UNUSED int res;
  char channel[1024];

  const char *my_master = MSG_process_get_data(MSG_process_self());
  build_channel_name(channel, my_master, MSG_host_get_name(MSG_host_self()));

  XBT_DEBUG("Receiving on channel \"%s\"", channel);

  while (1) {
    /* Send a request */
    msg_task_t request = MSG_task_create("request", 0, 0, MSG_host_self());
    MSG_task_send(request, my_master);

    res = MSG_task_receive(&(task),channel);
    xbt_assert(res == MSG_OK, "MSG_task_receive failed");
    
    XBT_DEBUG("Received \"%s\"", MSG_task_get_name(task));
    if (!strcmp(MSG_task_get_name(task), "finalize")) {
      MSG_task_destroy(task);
      break;
    }

    XBT_DEBUG("Processing \"%s\"", MSG_task_get_name(task));
    MSG_task_execute(task);
    XBT_DEBUG("\"%s\" done", MSG_task_get_name(task));
    MSG_task_destroy(task);
    task = NULL;
  }
  XBT_DEBUG("I'm done. See you!");
  return 0;
}                               /* end_of_worker */

/** Main function */
int main(int argc, char *argv[])
{
  msg_error_t res = MSG_OK;

  MSG_init(&argc, argv);
  xbt_assert(argc > 2, "Usage: %s platform_file deployment_file\n"
             "\tExample: %s msg_platform.xml msg_deployment.xml\n", argv[0], argv[0]);
  {                             /*  Simulation setting */
    MSG_create_environment(argv[1]);
  }
  {                             /*   Application deployment */
    MSG_function_register("master", master);
    MSG_function_register("worker", worker);
    MSG_launch_application(argv[2]);
  }
  res = MSG_main();

  XBT_INFO("Simulation time %g", MSG_get_clock());
  return (res != MSG_OK);
}                               /* end_of_main */

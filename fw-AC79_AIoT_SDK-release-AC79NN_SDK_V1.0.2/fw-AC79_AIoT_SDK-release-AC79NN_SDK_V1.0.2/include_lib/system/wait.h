#ifndef WAIT_COMPLETION_H
#define WAIT_COMPLETION_H


u16 wait_completion_add_to_task(const char *task_name, int (*condition)(void *), int (*callback)(void *), void *priv, void *condition_priv);

u16 wait_completion(int (*condition)(void *), int (*callback)(void *), void *priv, void *condition_priv);

int wait_completion_del(u16 id);


#endif

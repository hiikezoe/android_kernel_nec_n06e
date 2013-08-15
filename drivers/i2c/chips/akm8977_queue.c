/**********************************************************************
* File Name: drivers/i2c/chips/akm8977_queue.c
* 
* (C) NEC CASIO Mobile Communications, Ltd. 2013
**********************************************************************/






#include <linux/slab.h>
#include <linux/akm8977_queue.h>

#if defined(DEBUG)
#define DEBUG_TRACE(args...)    printk( KERN_INFO args)
#define DEBUG_ERR(args...)      printk( KERN_INFO "[Error!!] " args)
#else
#define DEBUG_TRACE(args...)
#define DEBUG_ERR(args...)
#endif

#define FALSE 0
#define TRUE  1


BOOL IsInit = FALSE;


static struct mutex queue_mutex;


T_QUEUE* head_queue_addr[E_QUEUE_KIND_MAX];

T_QUEUE* tail_queue_addr[E_QUEUE_KIND_MAX];





void ErrorQueueHandler( E_QUEUE_KIND kind )
{
    ClearQueue( kind );
}





void InitializeQueue()
{
    int count = 0;

    DEBUG_TRACE( "<InitializeQueue>\n" );

    if( IsInit != FALSE )
    {
        return;
    }

    IsInit = TRUE;

    mutex_init(&queue_mutex);

    for( count = 0; count < E_QUEUE_KIND_MAX; count++ )
    {
        head_queue_addr[count] = NULL;
        tail_queue_addr[count] = NULL;
    }

}





void TerminateQueue()
{
    int count;

    DEBUG_TRACE( "<TerminateQueue>\n" );

    if( IsInit == FALSE )
    {
        return;
    }

    for( count = 0; count < E_QUEUE_KIND_MAX; count++ )
    {
        ClearQueue( count );
    }

    IsInit = FALSE;
}





void ClearQueue( int kind )
{
    int ret;
    T_MSG_DATA dummy;

    DEBUG_TRACE( "<ClearQueue> kind(%d)\n",kind );

    if( IsInit == FALSE )
    {
        return;
    }

    for(;;)
    {
        ret = Dequeue( kind, &dummy );

        if( ret == QUE_EMPTY )
        {
            break;
        }
    }
}







int Enqueue( int kind, T_MSG_DATA info )
{
#if defined(DEBUG)
    int QueueCount = 0;
    T_QUEUE* Counttmp;
#endif

    T_QUEUE *new_queue = NULL;

    if( IsInit == FALSE )
    {
        return QUE_ERROR;
    }

    new_queue = (T_QUEUE *)kmalloc( sizeof(T_QUEUE) , GFP_KERNEL);

    if( new_queue == NULL )
    {
        return QUE_ERROR;
    }

    memset( new_queue, 0x0, sizeof(T_QUEUE) );

    memcpy( &(new_queue->info), &info, sizeof(T_MSG_DATA) );

    mutex_lock( &queue_mutex );

    if( head_queue_addr[kind] == NULL )
    {
        head_queue_addr[kind]       = new_queue;
        tail_queue_addr[kind]       = head_queue_addr[kind];
        tail_queue_addr[kind]->next = NULL;
    }
    else
    {
        tail_queue_addr[kind]->next = new_queue;
        tail_queue_addr[kind]       = new_queue;
        tail_queue_addr[kind]->next = NULL;
    }

#if defined(DEBUG)
    Counttmp = head_queue_addr[kind];
    for(;;)
    {
        if( Counttmp != NULL )
        {
            Counttmp = (T_QUEUE*)Counttmp->next;
            QueueCount++;
        }
        else
        {
            break;
        }
    }

    DEBUG_TRACE( "<Enque> kind(%d) Que Count=(%d)\n", kind, QueueCount );
#endif

    mutex_unlock( &queue_mutex );

    return QUE_OK;
}








int Dequeue( int kind, T_MSG_DATA* info )
{
#if defined(DEBUG)
    int QueueCount = 0;
    T_QUEUE* Counttmp;
#endif

    T_QUEUE* tmp;

    if( IsInit == FALSE )
    {
        return QUE_ERROR;
    }

    if( info == NULL )
    {
        return QUE_ERROR;
    }

    mutex_lock( &queue_mutex );

    if( head_queue_addr[kind] == NULL )
    {
        mutex_unlock( &queue_mutex );

        DEBUG_TRACE( "<Deque> kind(%d) QUE_EMPTY\n", kind );

        return QUE_EMPTY;
    }

    memcpy( info, &(head_queue_addr[kind]->info), sizeof(T_MSG_DATA) );

    tmp = head_queue_addr[kind];

    if( head_queue_addr[kind]->next != NULL )
    {
        head_queue_addr[kind] = (T_QUEUE*)head_queue_addr[kind]->next;
    }
    else
    {
        head_queue_addr[kind] = NULL;
        tail_queue_addr[kind] = NULL;
    }

    kfree(tmp);

#if defined(DEBUG)
    Counttmp = head_queue_addr[kind];
    for(;;)
    {
        if( Counttmp != NULL )
        {
            Counttmp = (T_QUEUE*)Counttmp->next;
            QueueCount++;
        }
        else
        {
            break;
        }
    }

    DEBUG_TRACE( "<Deque> kind(%d) Que Count=(%d)\n", kind, QueueCount );
#endif

    mutex_unlock( &queue_mutex );

    return QUE_OK;
}


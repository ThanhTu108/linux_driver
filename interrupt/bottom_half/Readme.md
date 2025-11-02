# Bottom half
When Interrupt triggers, Interrupt Handler should be executed very quickly and it should not run for more time (it should not perform time-consuming tasks). If we have the interrupt handler who is doing more tasks then we need to divide it into two halves.

Top Half: execute immediately when interrupt occurs (Interrupt Handler)
Bottom Half: execute later when system is idle (deferred work)

## Types of Bottom Half
1. **Work Queues**: execute in process context
2. **Tasklets**: 
3. **Softirqs**: execute in atomic context
4. **Threaded Interrupts**: execute in atomic context
In this example we are using Tasklets to implement Bottom Half.


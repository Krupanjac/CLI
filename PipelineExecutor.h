#ifndef PIPELINEEXECUTOR_H_
#define PIPELINEEXECUTOR_H_

#include <string>
#include "Parser.h"
#include "CommandFactory.h"

class PipelineExecutor {
public:
    PipelineExecutor() = default;
    ~PipelineExecutor() = default;

    void run(Parser& parser, PSIGN& prompt);

private:
    CommandFactory factory; // reused per loop
};

#endif // PIPELINEEXECUTOR_H_

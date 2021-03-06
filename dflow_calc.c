/* 046267 Computer Architecture - Winter 20/21 - HW #3               */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <stdbool.h>
#include <stdio.h>

#define MAX_DEP_NUM 2

// ======================================== My struct for saving program context ================================ //
typedef struct prog_context_t {
    int instNum;
    int totalDepth;
    int* depths;
    int** insts_with_deps;
} *ProgramContext;

// =========================================  Helper Functions  ============================================ //
static inline int max(int a, int b) {
    return a > b ? a : b;
}
static ProgramContext initCtx(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts) {
    ProgramContext ctx = (ProgramContext )malloc(sizeof(*ctx));
    if(!ctx){
        return PROG_CTX_NULL;
    }
    //allocate fields
    ctx->insts_with_deps = malloc(sizeof(int*)*numOfInsts); // array of arrays.
    if(!ctx->insts_with_deps) {
        free(ctx);
        return PROG_CTX_NULL;
    }
    ctx->depths = (int* )malloc(numOfInsts*sizeof(int)); //depths init to 0.
    if(!ctx->depths){
        free(ctx->insts_with_deps);
        free(ctx);
        return PROG_CTX_NULL;
    }
    //init allocated arrays
    for(int i = 0 ; i < (int )numOfInsts ; i++){
        ctx->insts_with_deps[i] = (int* )malloc(sizeof(int)*MAX_DEP_NUM);
        if(!ctx->insts_with_deps[i]){
            for(int j = 0 ; j < i ; j++){
                free(ctx->insts_with_deps[j]);
            }
            free(ctx->depths);
            free(ctx->insts_with_deps);
            free(ctx);
            return PROG_CTX_NULL;
        }
        ctx->insts_with_deps[i][0] = ctx->insts_with_deps[i][1] = -1; // init sons to default -1.
        ctx->depths[i] = 0;
    }
    ctx->instNum = (int )numOfInsts;
    ctx->totalDepth = 0;

    return ctx;
}
static void analyze(const unsigned int opsLatency[], const InstInfo progTrace[], ProgramContext ctx) {
    //analyze dependencies
    for(int curr_inst = 0 ; curr_inst < ctx->instNum ; curr_inst++) {
        bool found1 = false, found2 = false;
        for(int prev_inst = curr_inst-1 ; (prev_inst >= 0) && !(found1 && found2); prev_inst--){
            if((progTrace[prev_inst].dstIdx == progTrace[curr_inst].src1Idx) && !found1){
                //first son found
                ctx->insts_with_deps[curr_inst][0] = prev_inst;
                ctx->depths[curr_inst] = max(ctx->depths[curr_inst],(int )(ctx->depths[prev_inst] + opsLatency[progTrace[prev_inst].opcode]));
                found1 = true; // don't enter here again
                //update program depth
                ctx->totalDepth = max(ctx->totalDepth,(ctx->depths[curr_inst] + (int )opsLatency[progTrace[curr_inst].opcode]));
            }
            //same as first son.
            if((progTrace[prev_inst].dstIdx == progTrace[curr_inst].src2Idx) && !found2){
                ctx->insts_with_deps[curr_inst][1] = prev_inst;
                ctx->depths[curr_inst] = max(ctx->depths[curr_inst],(int )(ctx->depths[prev_inst] + opsLatency[progTrace[prev_inst].opcode]));
                found2 = true;
                ctx->totalDepth = max(ctx->totalDepth,(ctx->depths[curr_inst] + (int )opsLatency[progTrace[curr_inst].opcode]));
            }
        }
    }
}
// ====================================== Main Functions ============================================== //
ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts) {
    //init and analyze
    ProgramContext ctx = initCtx(opsLatency,progTrace,numOfInsts);
    if(!ctx) {
        return PROG_CTX_NULL;
    }
    analyze(opsLatency,progTrace,ctx);
    return ctx;
}

void freeProgCtx(ProgCtx ctx) {
    if(ctx){
        ProgramContext my_ctx = (ProgramContext )ctx;
        free(my_ctx->depths);
        for(int i = 0 ; i < my_ctx->instNum ; i++){
            free(my_ctx->insts_with_deps[i]);
        }
        free(my_ctx->insts_with_deps);
        free(ctx);
    }
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
    if(ctx) {
        ProgramContext my_ctx = (ProgramContext )ctx;
        if(theInst < 0 || theInst >= my_ctx->instNum){
            return -1;
        }
        return my_ctx->depths[theInst];
    }
    return -1;
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
    if(ctx) {
       ProgramContext my_ctx = (ProgramContext )ctx;
       if(theInst < 0 || theInst >= my_ctx->instNum){
           return -1;
       }
       //sons are saved in the matrix inside struct.
       *src1DepInst = my_ctx->insts_with_deps[theInst][0];
       *src2DepInst = my_ctx->insts_with_deps[theInst][1];
        return 0;
    }
    return -1;
}

int getProgDepth(ProgCtx ctx) {
    if(ctx){
        ProgramContext my_ctx = (ProgramContext )ctx;
        return my_ctx->totalDepth;
    }
    return 0;
}

/* ================================================ End Of Code ======================================== */
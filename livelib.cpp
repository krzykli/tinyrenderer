
#include <stdio.h>
#include "cr.h"

#include "render.h"
#include "types.h"
#include "debug.h"

CR_EXPORT int cr_main(cr_plugin *ctx, cr_op operation) {

    switch (operation) {
        case CR_LOAD:
            print("hot reload");
            return 0;
        case CR_UNLOAD:
            // if needed, save stuff to pass over to next instance
            print("unload dylib");
            return 0;
        case CR_CLOSE:
            return 0;
        case CR_STEP:
            trRender((App *)ctx->userdata);
            return 0;
    }
    return 0;
}


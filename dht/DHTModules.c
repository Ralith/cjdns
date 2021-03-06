/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <assert.h>
#include <string.h>

#include "memory/Allocator.h"
#include "memory/BufferAllocator.h"
#include "io/Reader.h"
#include "io/Writer.h"
#include "benc/Object.h"
#include "benc/serialization/BencSerializer.h"
#include "benc/serialization/standard/StandardBencSerializer.h"
#include "DHTModules.h"

#define DEBUG2(x, y)
/* #define DEBUG2(x, y) fprintf(stderr, x, y); fflush(stderr) */

/** This defines what format the registry will be serialized in. */
#define SERIALIZER StandardBencSerializer_get()

/*--------------------Prototypes--------------------*/
static void forEachModule(int (*doThis)(struct DHTModule* module, struct DHTMessage* message),
                          struct DHTMessage* message,
                          const struct DHTModuleRegistry* registry);
static inline void deserializeContext(struct DHTModule* module,
                                      struct DHTModuleRegistry* registry);

/*--------------------Interface--------------------*/

/** @see DHTModules.h */
struct DHTModuleRegistry* DHTModules_new(struct Allocator* allocator)
{
    struct DHTModuleRegistry* reg =
        allocator->calloc(sizeof(struct DHTModuleRegistry), 1, allocator);
    reg->allocator = allocator;
    reg->members = allocator->calloc(sizeof(char*), 1, allocator);
    return reg;
}

/** @see DHTModules.h */
int DHTModules_register(struct DHTModule* module,
                        struct DHTModuleRegistry* registry)
{
    deserializeContext(module, registry);

    registry->members = registry->allocator->realloc(registry->members,
                                                     sizeof(char*) * (registry->memberCount + 2),
                                                     registry->allocator);

    registry->members[registry->memberCount] = module;
    registry->memberCount++;
    registry->members[registry->memberCount] = NULL;

    return 0;
}

/** @see DHTModules.h */
void DHTModules_handleIncoming(struct DHTMessage* message,
                               const struct DHTModuleRegistry* registry)
{
    if (!(message && registry && registry->members && registry->memberCount)) {
        return;
    }

    struct DHTModule** firstModulePtr = registry->members;
    struct DHTModule** modulePtr = registry->members + registry->memberCount - 1;
    struct DHTModule* module;

    while (modulePtr >= firstModulePtr) {
        module = *modulePtr;
        if (module && module->handleIncoming) {
            DEBUG2("<< calling: %s->handleIncoming\n", module->name);
            if (module->handleIncoming(message, module->context) != 0) {
                // TODO: Call a debugger with all unhandlable messages?
                return;
            }
        } else {
            DEBUG2("<< skipping %s->handleIncoming\n", module->name);
        }
        modulePtr--;
    }
}

static int dhtModuleHandleOutgoing(struct DHTModule* module, struct DHTMessage* message)
{
    assert(module != NULL);
    if (module->handleOutgoing) {
        DEBUG2(">> calling: %s->handleOutgoing\n", module->name);
        return module->handleOutgoing(message, module->context);
    } else {
        DEBUG2(">> skipping: %s->handleOutgoing\n", module->name);
    }
    return 0;
}

/** @see DHTModules.h */
void DHTModules_handleOutgoing(struct DHTMessage* message,
                               const struct DHTModuleRegistry* registry)
{
    forEachModule(dhtModuleHandleOutgoing, message, registry);
}

/** @see DHTModules.h */
void DHTModules_serialize(const struct DHTModuleRegistry* registry,
                          const struct Writer* writer)
{
    char buffer[1024];
    struct Allocator* allocator = BufferAllocator_new(buffer, 1024);
    Dict* dictionary = Dict_new(allocator);

    struct DHTModule** modulePtr = registry->members;
    struct DHTModule* module = *modulePtr;
    while (module) {
        if (module->serialize != NULL) {
            Dict_putDict(dictionary,
                               String_new(module->name, allocator),
                               module->serialize(module->context),
                               allocator);
        }
        modulePtr++;
        module = *modulePtr;
    }
    SERIALIZER->serializeDictionary(writer, dictionary);
}

/** @see DHTModules.h */
struct DHTModuleRegistry* DHTModules_deserialize(const struct Reader* reader,
                                                 struct Allocator* allocator)
{
    Dict* dictionary = Dict_new(allocator);
    if (SERIALIZER->parseDictionary(reader, allocator, dictionary) != 0) {
        return NULL;
    }

    struct DHTModuleRegistry* reg = DHTModules_new(allocator);
    reg->serializedContexts = dictionary;
    return reg;

    return NULL;
}

/*----------------------Internals----------------------*/

/**
 * Deserialize the context for this module.
 * First the registry is deserialized then the modules are registered.
 * When the modules are registered, if they have serialized contexts,
 * those are deserialized by this function which calls their own
 * deserialization functions.
 *
 * @param module the module to deserialize the context for.
 * @param registry the DHT module registry.
 */
static inline void deserializeContext(struct DHTModule* module,
                                      struct DHTModuleRegistry* registry)
{
    char* name = (char*) module->name;
    if (module && registry && registry->serializedContexts) {
        Dict* serContext = Dict_getDict(registry->serializedContexts,
                                                 &(String) { .len = strlen(name), .bytes = name } );
        if (module->deserialize && module->context && serContext) {
            module->deserialize(serContext, module->context);
        }
    }
}

/**
 * Do something to every module which is registered.
 * @param doThis the callback.
 * @param registry state.
 */
static void forEachModule(int (*doThis)(struct DHTModule* module, struct DHTMessage* message),
                          struct DHTMessage* message,
                          const struct DHTModuleRegistry* registry)
{
    struct DHTModule** modulePtr = registry->members;
    struct DHTModule* module = *modulePtr;
    while (module) {
        if (doThis(module, message) != 0) {
            return;
        }
        modulePtr++;
        module = *modulePtr;
    }
}

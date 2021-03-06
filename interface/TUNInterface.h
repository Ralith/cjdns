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
#ifndef TUNInterface_H
#define TUNInterface_H

#include <event2/event.h>

#include "interface/Interface.h"
#include "benc/Object.h"
#include "memory/Allocator.h"

struct Interface* TUNInterface_new(String* interfaceName,
                                   struct event_base* base,
                                   struct Allocator* allocator);

#endif

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
#ifndef DOUBLE_INTERFACE_H
#define DOUBLE_INTERFACE_H

/**
 * Connect two interfaces together like a double male adapter.
 * Any traffic coming in on one will be forwarded to the other.
 */
struct DoubleInterface
{
    struct Interface a;
    struct Interface b;
};

struct DoubleInterface* DoubleInterface_new(struct Allocator* allocator);

#endif

#include "ranking_selection.h"
#include <cassert>
struct Entry{uint32_t id;int score;};
int main(){
 Entry items[]={{14,1},{72,2},{99,3}};
 uint32_t stale[]={14,555};
 assert(removeSelected(items,3,stale,2)==-1);assert(items[0].id==14 && items[1].id==72);
 uint32_t one[]={72};
 int n=removeSelected(items,3,one,1);assert(n==2);assert(items[0].id==14 && items[1].id==99);
 uint32_t both[]={99,14};assert(removeSelected(items,n,both,2)==0);
 Entry reordered[]={{99,3},{14,1},{72,2}};
 n=removeSelected(reordered,3,one,1);assert(n==2 && reordered[0].id==99 && reordered[1].id==14);
 uint32_t duplicate[]={99,99};assert(removeSelected(reordered,n,duplicate,2)==1);
 assert(reordered[0].id==14);assert(removeSelected(reordered,1,one,0)==-1);
}

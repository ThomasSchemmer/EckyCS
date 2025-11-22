#include "ItemRenderData.h"

#include "../ItemEntity.h"
#include "../Components/ItemComponent.h"
#include "EckyCS/Util/TypeInfo.h"

void ItemRenderData::Create(size_t InCount)
{
    RenderData::Create(InCount);
    
    glGenBuffers(1, &ItemBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, ItemBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ItemComponent) * InCount, nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(4, 1, GL_INT, GL_FALSE, sizeof(int), (void*)0);
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1); 

    // need to add +1 as the resulting View always includes the EntityID as well, which is not in the ReqCompss
    constexpr size_t ItemIndex = GetIndexOf<ItemComponent, ItemEntity::RequiredComponents>() + 1;
    ParamLookup.emplace(typeid(ItemComponent), make_tuple(ItemBuffer, sizeof(ItemComponent), ItemIndex));
}

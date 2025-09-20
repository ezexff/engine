internal void
UpdateAndRenderPhysics3(game_memory *Memory, game_input *Input)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    
    memory_arena *ConstArena = &GameState->ConstArena;
    memory_arena *TranArena = &TranState->TranArena;
    
    renderer_frame *Frame = &Memory->Frame;
    renderer *Renderer = (renderer *)Frame->Renderer;
    
    mode_physics3 *ModePhysics3 = &GameState->ModePhysics3;
    if(!ModePhysics3->IsInitialized)
    {
        ModePhysics3->ExplodeRadius = 100.0f;
        
        random_series Series = RandomSeed(300);
        u32 RowCount = 0;
        u32 ColumnCount = 0;
        for(u32 Index = 0;
            Index < ArrayCount(ModePhysics3->EntityArray);
            ++Index)
        {
            test_entity *Entity = ModePhysics3->EntityArray + Index;
            Entity->Type = TestEntityType_Circle;
            r32 R = RandomUnilateral(&Series);
            r32 G = RandomUnilateral(&Series);
            r32 B = RandomUnilateral(&Series);
            Entity->Color = V4(R, G, B, 1.0f);
            Entity->OutlineColor = V4(1, 1, 1, 1);
            
            if(Index % 10 == 0){RowCount++; ColumnCount = 0;}
            Entity->P = V2((r32)(700 + 60 * ColumnCount), 100 + 60 * (r32)RowCount);
            ColumnCount++;
            Entity->Size = (r32)RandomBetween(&Series, 40, 60);
            Entity->Radius = Entity->Size / 2.0f;
            
            Entity->Density = 1.2f;
            // NOTE(ezexff): mass = area * depth * density
            Entity->Mass = Entity->Size * Entity->Density;
            Entity->ForceMagnitude = 10.0f;
        }
        
        ModePhysics3->IsInitialized = true;
    }
    
    // NOTE(ezexff): inputs
    if(WasPressed(Input->MouseButtons[PlatformMouseButton_Left]))
    {
        // TODO(ezexff): explosion
        r32 ExplodeRadius = ModePhysics3->ExplodeRadius;
        v2 MouseP = V2(Input->MouseP.x, Frame->Dim.y - Input->MouseP.y);
        
        for(u32 Index = 0;
            Index < ArrayCount(ModePhysics3->EntityArray);
            ++Index)
        {
            test_entity *Entity = ModePhysics3->EntityArray + Index;
            v2 RelP = Entity->P - MouseP;
            r32 Len = Length(RelP);
            if(Len < ExplodeRadius)
            {
                r32 K = 1.0f - Len / ExplodeRadius;
                r32 A = 100.0f;
                // NOTE(ezexff): вектор направления после взрыва
                Entity->ddP = Normalize(RelP) * K * A;
                Entity->OutlineColor = V4(1, 0, 0, 1);
            }
        }
    }
    
    // NOTE(ezexff): move
    r32 dt = Input->dtForFrame;
    for(u32 EntityIndex = 0;
        EntityIndex < ArrayCount(ModePhysics3->EntityArray);
        ++EntityIndex)
    {
        test_entity *Entity = ModePhysics3->EntityArray + EntityIndex;
        
        /* 
                Entity->Force = Entity->ddP * Entity->ForceMagnitude;
                v2 Delta = Entity->Force / Entity->Mass;
                Entity->dP += Delta  * dt;
                Entity->P += Entity->dP;
                 */
        Entity->P += Entity->ddP * dt;
        Entity->Force = {};
    }
    
    // NOTE(ezexff): draw
    v4 OutlineColor = V4(1, 0, 0, 1);
    PushCircleOutlineOnScreen(&Renderer->PushBufferUI, V2(Input->MouseP.x, Frame->Dim.y - Input->MouseP.y), ModePhysics3->ExplodeRadius, 2, OutlineColor, 10001);
    for(u32 Index = 0;
        Index < ArrayCount(ModePhysics3->EntityArray);
        ++Index)
    {
        test_entity *Entity = ModePhysics3->EntityArray + Index;
        
        switch(Entity->Type)
        {
            case TestEntityType_Rect:
            {
                //
            } break;
            
            case TestEntityType_Circle:
            {
                PushCircleOnScreen(&Renderer->PushBufferUI, Entity->P, Entity->Radius, Entity->Color, 10000);
                PushCircleOutlineOnScreen(&Renderer->PushBufferUI, Entity->P, Entity->Radius, 3, Entity->OutlineColor, 10000);
            } break;
            
            InvalidDefaultCase;
        }
    }
}
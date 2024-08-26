inline low_entity *
GetLowEntity(mode_world *ModeWorld, u32 Index)
{
    low_entity *Result = 0;
    
    if((Index > 0) && (Index < ModeWorld->LowEntityCount))
    {
        Result = ModeWorld->LowEntities + Index;
    }
    
    return(Result);
}

inline move_spec
DefaultMoveSpec(void)
{
    move_spec Result;
    
    Result.UnitMaxAccelVector = false;
    Result.Speed = 1.0f;
    Result.Drag = 0.0f;
    
    return(Result);
}
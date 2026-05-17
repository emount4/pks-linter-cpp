int leakMemory()
{
    int* p = new int;
    return *p;
}

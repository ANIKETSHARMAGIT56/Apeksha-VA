an event is constructed as follows

```json
{
    "emmiter":---,
    "target":---,
    "name":---,
    "data":{
        "feild_1":---,
        "feild_2":---,
        .
        .
        .
        "feild_n":---,
    }
}
```

```c++
struct event{
    std::string emmiter;
    std::string target;
    std::string name;
    std::unordered_map data{
        {"feild1":---},
        {"feild2":---},
        .
        .
        .
        {"feildn":---},
    }
}
```
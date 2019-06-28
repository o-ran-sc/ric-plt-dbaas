Build Image for dbaas-ha
------------------------
```
docker build -t <repository>:<tag> -f docker/Dockerfile.redis-ha .
docker push <repository>:<tag>
```

Clients for dbaas-ha
--------------------
* Clients should discover their master node
  - Refer to [Guidelines for Redis clients with support for Redis Sentinel](
   https://redis.io/topics/sentinel-clients)

* For the Discovery Service which utilizes the Redis Sentinel API, the following clients are recommended:
  - Redis-py (Python redis client)
  - HiRedis (C redis client)
  - Jedis (Java redis client)
  - Ioredis (NodeJS redis client)


Install helm chart for dbaas-ha
-------------------------------
```
helm install dbaas-ha-service --name r2
```

Delete helm chart for dbaas-ha
------------------------------
```
helm delete r2 --purge
```



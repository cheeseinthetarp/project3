#start nginx
sudo /usr/local/nginx/sbin/nginx

#start fastDFS
sudo /usr/bin/fdfs_trackerd /etc/fdfs/tracker.conf
sudo /usr/bin/fdfs_storaged /etc/fdfs/storage.conf

#start redis-ser
redis-server /home/cookie/project3/redis-3.2.1/redis.conf

spawn-fcgi -a 127.0.0.1 -p 8081 -f ./bin/upload
spawn-fcgi -a 127.0.0.1 -p 8085 -f ./bin/data



host_num=32
NODE_NAME=node
USER=root
host_address=192.168.0.

for((i=1; i<=$host_num; i++));
do
{
        if [[ $i -ge 0 && $i -lt 10 ]]
        then
                host=${NODE_NAME}0${i}
        else
                host=${NODE_NAME}$i
        fi
        host_ip=${host_address}$i
        ssh $host "tc qdisc delete dev eth0 root"

}
done


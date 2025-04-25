#include <unordered_map>
#include "storage.h"

// ==============
// Внутренний функционал узла
// ==============

enum StatusEnum {
    stopped,
    working,
    ready_to_reconfigure,
    reconfiguring
};

static StatusEnum current_server_status;

// Список узлов кластера
static std::vector<NodeIpAddress>;

// Соответствие файлов и их метаданных (дата создания, хэш сумма ...) 
static unordered_map<FileName, FileMetaData> files_metadata;

// Карта частей файлов, показывает какие части каких файлов лежат на каком узле
static unordered_map<NodeIpAddress, std::vector<FilePart>> file_parts_map;


StatusEnum check_if_node_alive(NodeIpAddress);

// Опрашивает узлы кластера, дабы убедиться в целостности
bool check_if_cluster_consistent()

// Вызывается в двух случаях:
// 1. если check_if_cluster_consistent вернул false (из кластера выпал узел)
// 2. к данному узлу присоединился новый узел, этот узел запускает реконфигурацию всего кластера
void calculate_new_file_parts_map(int nodes_count){
    ...
    file_parts_map = ... // вставить откуда берёт новую карту
    ...
}

// оповещает узел о необходимости реконфигурациио
void alert_node_that_reconfiguration_needed(NodeIpAddress);

// оповещает кластер о необходимости реконфигурации, по конкретной карте распределения фрагментов файлов
void alert_cluster_that_reconfiguration_needed(file_parts_map);
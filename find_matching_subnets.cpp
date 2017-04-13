#include <iostream>
#include <list>
#include <new>

using namespace std;

#define   TRUE   1
#define   FALSE  0

typedef struct subnet {
    int       ip_address;
    int       netmask;
} Subnet;

typedef list<Subnet *> subnetList;

typedef struct node {
    struct node    *left;
    struct node    *right;
    bool           isLeaf;
} BinaryTreeNode;

BinaryTreeNode  *newBinaryTreeNode(void);
void            parseIP_address_and_mask(char *, Subnet *);
void            populateSubnetInBinaryTree(BinaryTreeNode *, Subnet *, int);
void            searchSubnetInBinaryTree(BinaryTreeNode *, Subnet *, int, subnetList *);

int main() 
{
    int    i;
    Subnet insertSubnet, searchSubnet;
    subnetList::iterator iter;

    char *subnetToMatch = "1.2.3.128/28";

    subnetList *p_subnets = new subnetList;

    char *ip_addresses_and_masks[6] = {
        "1.2.3.20/24",    // Match (superset)
        "1.3.2.100/16",   // Not a match
        "1.2.3.255/31",   // Match (subset)
        "1.2.3.150/27",   // Match (superset)
        "1.2.3.150/28",   // Not a match
        "1.2.3.70/25"     // Not a match
    };

    BinaryTreeNode *head = newBinaryTreeNode();

    for (i = 0; i < 6; i++) {
        cout << endl << "Traversing i = " << i << " entry: ";
        memset(&insertSubnet, 0, sizeof(insertSubnet));
        parseIP_address_and_mask(ip_addresses_and_masks[i], &insertSubnet);

        if (insertSubnet.netmask == 0) {
            // Special case: 0.0.0.0/0 subnet
            head->isLeaf = TRUE;
        }

        populateSubnetInBinaryTree(head, &insertSubnet, 31);
    }

    //cout << endl << "Parsing the search subnet ";
    memset(&searchSubnet, 0, sizeof(searchSubnet));
    parseIP_address_and_mask(subnetToMatch, &searchSubnet);
   
    //cout << endl << "Starting the search";
    searchSubnetInBinaryTree(head, &searchSubnet, 31, p_subnets); 

    i = 0;
    for (iter = p_subnets->begin(); iter != p_subnets->end(); iter++) {
        cout << endl << "A matching netmask is 0x" << hex << (*iter)->ip_address;
        cout << " and the number of bits in the netmask are " << dec << (*iter)->netmask << endl;
        i++;
    } 

    cout << endl; 
    return (0);
}


    
void  searchSubnetInBinaryTree(BinaryTreeNode *p_node, Subnet 
    *p_search_subnet, int shiftBits, subnetList *p_subnets)
{
    Subnet *matched_subnet;
    int    shiftMask;
    int    val; 

    if (NULL == p_node) { return; }

    if (p_node->isLeaf == TRUE) {

        matched_subnet = new Subnet;

        // e.g. if shiftBits is 30 then shiftMask will be 11000000 00000000 00000000 00000000 
        shiftMask = (1 << (31 - shiftBits)) - 1;
        shiftMask = shiftMask << (shiftBits + 1);

        matched_subnet->ip_address = p_search_subnet->ip_address & shiftMask;
        matched_subnet->netmask    = 31 - shiftBits;
            
        p_subnets->push_back(matched_subnet);
    }

    if (shiftBits == (32 - p_search_subnet->netmask)) {

        //cout << "Processed all superSet nodes" << endl;
        return;
    }

    val = (p_search_subnet->ip_address >> shiftBits) & (0x1);
    cout << val << " ";

    shiftBits--;

    if (val == 0) {
        searchSubnetInBinaryTree(p_node->left, p_search_subnet, shiftBits, p_subnets); 
    } else {
        searchSubnetInBinaryTree(p_node->right, p_search_subnet, shiftBits, p_subnets); 
    }
}

void  populateSubnetInBinaryTree(BinaryTreeNode *p_node, Subnet *p_subnet, int shiftBits)
{
    int val = (p_subnet->ip_address >> shiftBits) & (0x1);
    cout << val << " ";

    if (val == 0) {
        if (!p_node->left) { p_node->left = newBinaryTreeNode(); }
        if (shiftBits == (32 - p_subnet->netmask)) {
            p_node->left->isLeaf = TRUE;
            return;
        }
        p_node = p_node->left;

    } else {

        if (!p_node->right) { p_node->right = newBinaryTreeNode(); }
        if (shiftBits == (32 - p_subnet->netmask)) {
            p_node->right->isLeaf = TRUE;
            return; 
        }
        p_node = p_node->right;
    }

    shiftBits--;
    populateSubnetInBinaryTree(p_node, p_subnet, shiftBits);   
}

BinaryTreeNode *newBinaryTreeNode(void)
{
    BinaryTreeNode *newNode = (BinaryTreeNode *)malloc(sizeof(BinaryTreeNode));
    newNode->left   = NULL;
    newNode->right  = NULL;
    newNode->isLeaf = FALSE;
} 

void   parseIP_address_and_mask(char *ptr, Subnet *p_subnet)
{
    int decimals          = 3;      // max is 3 chars e.g. 127
    char *start_delimiter = ptr;
    char *end_delimiter   = NULL;
    char octet[4];
    int  int_octet        = 0;
    int  binary_octet     = 0;
    int  shift;

    p_subnet->ip_address = 0;

    while (decimals > 0) {
        memset(octet, '\0', 4); // max is 3 chars e.g. 127
        end_delimiter        = strchr(start_delimiter, '.');
        strncpy(octet, start_delimiter, end_delimiter - start_delimiter);

        int_octet            = atoi(octet);

        binary_octet = 0;
        shift        = 8 * decimals;
        while (int_octet) {
            if (int_octet % 2) {
                p_subnet->ip_address = p_subnet->ip_address | (1 << shift);
            }

            shift     = shift + 1;
            int_octet = int_octet / 2;
        }            

        start_delimiter      = end_delimiter + 1;        
        decimals--;
    }
  
    memset(octet, '\0', 4); // max is 3 chars e.g. 127
    end_delimiter        = strchr(start_delimiter, '/');
    strncpy(octet, start_delimiter, end_delimiter - start_delimiter);

    int_octet            = atoi(octet);

    binary_octet = 0;

    shift        = 8 * decimals;
    while (int_octet) {
        if (int_octet % 2) {
            p_subnet->ip_address = p_subnet->ip_address | (1 << shift);
        }

        shift     = shift + 1;
        int_octet = int_octet / 2;
    }            
   
    //cout << endl << "The ip address in hexadecimal is 0x" << hex << p_subnet->ip_address << endl;

    start_delimiter = end_delimiter + 1;
    while ('\0' != (*end_delimiter)) {
        end_delimiter++; 
    }  

    memset(octet, '\0', 4); // max is 3 chars e.g. 127
    strncpy(octet, start_delimiter, end_delimiter - start_delimiter);
    int_octet            = atoi(octet);
    p_subnet->netmask    = int_octet;

    //cout << "The netmask for the subnet is " << dec << p_subnet->netmask << endl;
}

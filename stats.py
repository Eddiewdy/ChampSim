import re

def analyze_large_objects(object_file_path, size_file_path):
    # Read the OBJECT_ID and ADDRESS STRIDE data
    with open(object_file_path, 'r') as file:
        object_content = file.read()
    
    # Read the Range ID and Total Size data
    with open(size_file_path, 'r') as file:
        size_content = file.read()

    # Extract ADDRESS STRIDE values associated with OBJECT_ID
    object_data = re.findall(r'OBJECT_ID:.*?ADDRESS STRIDE:([\d\s\-]+)', object_content)

    # Extract and filter Total Size data associated with Range ID, considering only sizes greater than 4KB
    size_data = re.findall(r'Range ID: (\d+) \| Total Size: (\d+) bytes', size_content)
    filtered_size_data = {range_id: int(size) for range_id, size in size_data if int(size) > 4096}
    print(filtered_size_data)
    # Initialize counters
    count_above_64 = 0
    count_below_64 = 0

    object_strides = re.findall(r'OBJECT_ID:\s*(\d+).*?ADDRESS STRIDE:([\d\s\-]+)', object_content)

    # Check if the Range ID corresponding to an OBJECT_ID has a large enough Total Size
    for object_id, stride_values in object_strides:
        # If the OBJECT_ID's Range ID is in the filtered size data, analyze the strides
        if object_id in filtered_size_data:
            numbers = list(map(int, stride_values.split()))

            # Count the number of strides above and below 64
            for number in numbers:
                if abs(number) > 64:
                    count_above_64 += 1
                else:
                    count_below_64 += 1

    # Calculate the total number of strides and their ratios
    total_strides = count_above_64 + count_below_64
    ratio_above_64 = count_above_64 / total_strides if total_strides > 0 else 0
    ratio_below_64 = count_below_64 / total_strides if total_strides > 0 else 0

    return {
        "Total Strides": total_strides,
        "Above 64": count_above_64,
        "Below 64": count_below_64,
        "Ratio Above 64": ratio_above_64,
        "Ratio Below 64": ratio_below_64
    }

# Example of how to run the function with your file paths
if __name__ == "__main__":
    # /mnt/disk/wangyidi/centroid/out_3m
    # /mnt/disk/wangyidi/github/ChampSim/out_xml3m_ipcp_new
    object_file_path = '/mnt/disk/wangyidi/github/ChampSim/out_omne3m_ipstride_new'  # Change this to the path of your OBJECT_ID file
    size_file_path = '/mnt/disk/wangyidi/github/ChampSim/tools/omne_141b40m.out'  # Change this to the path of your Total Size file
    # object_file_path = '/mnt/disk/wangyidi/github/ChampSim/out_xml3m_ipcp_new'  # Change this to the path of your OBJECT_ID file
    # size_file_path = '/mnt/disk/wangyidi/centroid/out_3m'  # Change this to the path of your Total Size file
    result = analyze_large_objects(object_file_path, size_file_path)
    print(result)

char predef[] = {
    35, 32, 99, 104, 97, 114, 32, 115, 101, 116, 115, 10, 97, 110, 121, 32, 
    32, 32, 32, 32, 32, 32, 61, 32, 48, 32, 46, 46, 32, 50, 53, 53, 
    32, 35, 32, 65, 110, 121, 32, 99, 104, 97, 114, 97, 99, 116, 101, 114, 
    32, 105, 110, 32, 116, 104, 101, 32, 97, 108, 112, 104, 97, 98, 101, 116, 
    46, 10, 97, 115, 99, 105, 105, 32, 32, 32, 32, 32, 61, 32, 48, 32, 
    46, 46, 32, 49, 50, 55, 32, 35, 32, 65, 115, 99, 105, 105, 32, 99, 
    104, 97, 114, 97, 99, 116, 101, 114, 115, 46, 32, 48, 46, 46, 49, 50, 
    55, 10, 101, 120, 116, 101, 110, 100, 32, 32, 32, 32, 61, 32, 48, 32, 
    46, 46, 32, 50, 53, 53, 32, 35, 32, 65, 115, 99, 105, 105, 32, 101, 
    120, 116, 101, 110, 100, 101, 100, 32, 99, 104, 97, 114, 97, 99, 116, 101, 
    114, 115, 46, 32, 84, 104, 105, 115, 32, 105, 115, 32, 116, 104, 101, 32, 
    114, 97, 110, 103, 101, 32, 45, 49, 50, 56, 46, 46, 49, 50, 55, 32, 
    102, 111, 114, 32, 115, 105, 103, 110, 101, 100, 32, 97, 108, 112, 104, 97, 
    98, 101, 116, 115, 32, 97, 110, 100, 32, 116, 104, 101, 32, 114, 97, 110, 
    103, 101, 32, 48, 46, 46, 50, 53, 53, 32, 102, 111, 114, 32, 117, 110, 
    115, 105, 103, 110, 101, 100, 32, 97, 108, 112, 104, 97, 98, 101, 116, 115, 
    46, 10, 108, 111, 119, 101, 114, 32, 32, 32, 32, 32, 61, 32, 39, 97, 
    39, 32, 46, 46, 32, 39, 122, 39, 32, 35, 32, 76, 111, 119, 101, 114, 
    99, 97, 115, 101, 32, 99, 104, 97, 114, 97, 99, 116, 101, 114, 115, 46, 
    32, 91, 97, 45, 122, 93, 10, 117, 112, 112, 101, 114, 32, 32, 32, 32, 
    32, 61, 32, 39, 65, 39, 32, 46, 46, 32, 39, 90, 39, 32, 35, 32, 
    85, 112, 112, 101, 114, 99, 97, 115, 101, 32, 99, 104, 97, 114, 97, 99, 
    116, 101, 114, 115, 46, 32, 91, 65, 45, 90, 93, 10, 100, 105, 103, 105, 
    116, 32, 32, 32, 32, 32, 61, 32, 39, 48, 39, 32, 46, 46, 32, 39, 
    57, 39, 32, 35, 32, 68, 105, 103, 105, 116, 115, 46, 32, 91, 48, 45, 
    57, 93, 10, 97, 108, 112, 104, 97, 32, 32, 32, 32, 32, 61, 32, 117, 
    112, 112, 101, 114, 32, 124, 32, 108, 111, 119, 101, 114, 32, 35, 32, 65, 
    108, 112, 104, 97, 98, 101, 116, 105, 99, 32, 99, 104, 97, 114, 97, 99, 
    116, 101, 114, 115, 46, 32, 91, 65, 45, 90, 97, 45, 122, 93, 10, 97, 
    108, 110, 117, 109, 32, 32, 32, 32, 32, 61, 32, 100, 105, 103, 105, 116, 
    32, 124, 32, 97, 108, 112, 104, 97, 32, 35, 32, 65, 108, 112, 104, 97, 
    32, 110, 117, 109, 101, 114, 105, 99, 115, 46, 32, 91, 48, 45, 57, 65, 
    45, 90, 97, 45, 122, 93, 10, 120, 100, 105, 103, 105, 116, 32, 32, 32, 
    32, 61, 32, 100, 105, 103, 105, 116, 32, 124, 32, 39, 65, 39, 32, 46, 
    46, 32, 39, 70, 39, 32, 124, 32, 39, 97, 39, 32, 46, 46, 32, 39, 
    102, 39, 32, 35, 32, 72, 101, 120, 97, 100, 101, 99, 105, 109, 97, 108, 
    32, 100, 105, 103, 105, 116, 115, 46, 32, 91, 48, 45, 57, 65, 45, 70, 
    97, 45, 102, 93, 10, 99, 110, 116, 114, 108, 32, 32, 32, 32, 32, 61, 
    32, 48, 32, 46, 46, 32, 51, 49, 32, 35, 32, 67, 111, 110, 116, 114, 
    111, 108, 32, 99, 104, 97, 114, 97, 99, 116, 101, 114, 115, 46, 32, 48, 
    46, 46, 51, 49, 10, 103, 114, 97, 112, 104, 32, 32, 32, 32, 32, 61, 
    32, 39, 33, 39, 32, 46, 46, 32, 39, 126, 39, 32, 35, 32, 71, 114, 
    97, 112, 104, 105, 99, 97, 108, 32, 99, 104, 97, 114, 97, 99, 116, 101, 
    114, 115, 46, 32, 91, 33, 45, 126, 93, 10, 112, 114, 105, 110, 116, 32, 
    32, 32, 32, 32, 61, 32, 39, 32, 39, 32, 46, 46, 32, 39, 126, 39, 
    32, 35, 32, 80, 114, 105, 110, 116, 97, 98, 108, 101, 32, 99, 104, 97, 
    114, 97, 99, 116, 101, 114, 115, 46, 32, 91, 32, 45, 126, 93, 10, 112, 
    117, 110, 99, 116, 32, 32, 32, 32, 32, 61, 32, 39, 33, 39, 32, 46, 
    46, 32, 39, 47, 39, 32, 124, 32, 39, 58, 39, 32, 46, 46, 32, 39, 
    64, 39, 32, 124, 32, 39, 93, 39, 32, 46, 46, 32, 39, 96, 39, 32, 
    124, 32, 39, 123, 39, 32, 46, 46, 32, 39, 126, 39, 32, 35, 32, 80, 
    117, 110, 99, 116, 117, 97, 116, 105, 111, 110, 46, 32, 71, 114, 97, 112, 
    104, 105, 99, 97, 108, 32, 99, 104, 97, 114, 97, 99, 116, 101, 114, 115, 
    32, 116, 104, 97, 116, 32, 97, 114, 101, 32, 110, 111, 116, 32, 97, 108, 
    112, 104, 97, 110, 117, 109, 101, 114, 105, 99, 115, 46, 10, 110, 117, 108, 
    108, 32, 32, 32, 32, 32, 32, 61, 32, 48, 10, 116, 97, 98, 32, 32, 
    32, 32, 32, 32, 32, 61, 32, 57, 32, 32, 35, 32, 116, 97, 98, 117, 
    108, 97, 116, 105, 111, 110, 10, 108, 102, 32, 32, 32, 32, 32, 32, 32, 
    32, 61, 32, 49, 48, 32, 35, 32, 108, 105, 110, 101, 32, 102, 101, 101, 
    100, 10, 118, 101, 114, 116, 95, 116, 97, 98, 32, 32, 61, 32, 49, 49, 
    32, 35, 32, 118, 101, 114, 116, 105, 99, 97, 108, 32, 116, 97, 98, 10, 
    102, 102, 32, 32, 32, 32, 32, 32, 32, 32, 61, 32, 49, 50, 32, 35, 
    32, 102, 111, 114, 109, 32, 102, 101, 101, 100, 10, 99, 114, 32, 32, 32, 
    32, 32, 32, 32, 32, 61, 32, 49, 51, 32, 35, 32, 99, 97, 114, 114, 
    105, 97, 103, 101, 32, 114, 101, 116, 117, 114, 110, 10, 101, 111, 108, 32, 
    32, 32, 32, 32, 32, 32, 61, 32, 99, 114, 63, 32, 108, 102, 32, 35, 
    32, 117, 110, 105, 120, 32, 111, 114, 32, 119, 105, 110, 100, 111, 119, 36, 
    32, 101, 110, 100, 32, 108, 105, 110, 101, 10, 115, 112, 97, 99, 101, 32, 
    32, 32, 32, 32, 61, 32, 116, 97, 98, 32, 124, 32, 108, 102, 32, 124, 
    32, 118, 101, 114, 116, 95, 116, 97, 98, 32, 124, 32, 102, 102, 32, 124, 
    32, 99, 114, 32, 124, 32, 39, 32, 39, 32, 35, 32, 91, 92, 92, 116, 
    92, 92, 118, 92, 92, 102, 92, 92, 110, 92, 92, 114, 32, 93, 10, 104, 
    115, 112, 97, 99, 101, 32, 32, 32, 32, 61, 32, 116, 97, 98, 32, 124, 
    32, 39, 32, 39, 32, 35, 32, 91, 92, 92, 116, 92, 92, 118, 92, 92, 
    102, 92, 92, 110, 92, 92, 114, 32, 93, 10, 122, 108, 101, 110, 32, 32, 
    32, 32, 32, 32, 61, 32, 39, 39, 32, 35, 32, 90, 101, 114, 111, 32, 
    108, 101, 110, 103, 116, 104, 32, 115, 116, 114, 105, 110, 103, 10, 98, 97, 
    115, 101, 54, 52, 32, 32, 32, 32, 61, 32, 97, 108, 110, 117, 109, 32, 
    124, 32, 39, 43, 39, 32, 124, 32, 39, 47, 39, 32, 124, 32, 39, 61, 
    39, 10, 98, 97, 99, 107, 115, 108, 97, 115, 104, 32, 61, 32, 39, 92, 
    39, 10, 101, 110, 100, 32, 32, 32, 32, 32, 32, 32, 61, 32, 123, 32, 
    101, 110, 100, 58, 32, 105, 110, 112, 95, 99, 111, 110, 116, 32, 61, 32, 
    38, 38, 101, 110, 100, 59, 32, 114, 101, 116, 117, 114, 110, 32, 102, 97, 
    108, 115, 101, 59, 32, 125, 32, 97, 110, 121, 42, 42, 32, 35, 10, 10, 
    97, 110, 121, 95, 117, 116, 102, 56, 32, 32, 61, 32, 40, 32, 48, 32, 
    46, 46, 32, 49, 50, 55, 32, 41, 32, 124, 10, 32, 32, 32, 32, 32, 
    32, 32, 32, 32, 32, 32, 32, 40, 32, 49, 50, 56, 32, 46, 46, 32, 
    50, 50, 51, 32, 97, 110, 121, 32, 41, 32, 124, 10, 32, 32, 32, 32, 
    32, 32, 32, 32, 32, 32, 32, 32, 40, 32, 50, 50, 52, 32, 46, 46, 
    32, 50, 51, 57, 32, 97, 110, 121, 32, 97, 110, 121, 32, 41, 32, 124, 
    10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 40, 32, 50, 
    52, 48, 32, 46, 46, 32, 50, 53, 53, 32, 97, 110, 121, 32, 97, 110, 
    121, 32, 97, 110, 121, 32, 41, 10, 10, 35, 32, 109, 97, 110, 100, 97, 
    116, 111, 114, 121, 32, 97, 114, 103, 58, 32, 118, 97, 108, 32, 40, 101, 
    46, 103, 46, 32, 117, 105, 110, 116, 91, 32, 99, 111, 110, 116, 101, 110, 
    116, 95, 108, 101, 110, 103, 116, 104, 32, 93, 41, 10, 117, 105, 110, 116, 
    91, 32, 118, 97, 108, 44, 32, 116, 121, 112, 101, 32, 61, 32, 39, 117, 
    110, 115, 105, 103, 110, 101, 100, 32, 108, 111, 110, 103, 39, 32, 93, 32, 
    61, 10, 32, 32, 32, 32, 97, 100, 100, 95, 97, 116, 116, 114, 91, 32, 
    39, 116, 121, 112, 101, 32, 118, 97, 108, 59, 39, 32, 39, 115, 105, 112, 
    101, 95, 100, 97, 116, 97, 45, 62, 118, 97, 108, 32, 61, 32, 48, 39, 
    32, 93, 10, 32, 32, 32, 32, 100, 105, 103, 105, 116, 32, 123, 32, 115, 
    105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 118, 97, 108, 32, 61, 32, 
    42, 100, 97, 116, 97, 32, 45, 32, 39, 48, 39, 59, 32, 125, 10, 32, 
    32, 32, 32, 40, 32, 100, 105, 103, 105, 116, 32, 123, 32, 115, 105, 112, 
    101, 95, 100, 97, 116, 97, 45, 62, 118, 97, 108, 32, 61, 32, 49, 48, 
    32, 42, 32, 115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 118, 97, 
    108, 32, 43, 32, 40, 32, 42, 100, 97, 116, 97, 32, 45, 32, 39, 48, 
    39, 32, 41, 59, 32, 125, 32, 41, 42, 42, 10, 10, 35, 32, 112, 114, 
    105, 110, 116, 32, 97, 32, 99, 104, 97, 114, 10, 112, 91, 32, 109, 115, 
    103, 32, 61, 32, 39, 39, 32, 93, 32, 61, 10, 32, 32, 32, 32, 97, 
    100, 100, 95, 112, 114, 101, 108, 91, 32, 39, 35, 105, 102, 100, 101, 102, 
    32, 83, 73, 80, 69, 95, 77, 65, 73, 78, 39, 32, 93, 10, 32, 32, 
    32, 32, 97, 100, 100, 95, 112, 114, 101, 108, 91, 32, 39, 35, 105, 110, 
    99, 108, 117, 100, 101, 32, 60, 105, 111, 115, 116, 114, 101, 97, 109, 62, 
    39, 32, 93, 10, 32, 32, 32, 32, 97, 100, 100, 95, 112, 114, 101, 108, 
    91, 32, 39, 35, 101, 110, 100, 105, 102, 32, 47, 47, 32, 83, 73, 80, 
    69, 95, 77, 65, 73, 78, 39, 32, 93, 10, 32, 32, 32, 32, 123, 32, 
    115, 116, 100, 58, 58, 99, 111, 117, 116, 32, 60, 60, 32, 34, 109, 115, 
    103, 34, 32, 60, 60, 32, 115, 116, 100, 58, 58, 101, 110, 100, 108, 59, 
    32, 125, 10, 10, 35, 32, 112, 114, 105, 110, 116, 32, 34, 118, 97, 108, 
    32, 45, 62, 32, 36, 115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 
    118, 97, 108, 34, 10, 105, 91, 32, 118, 97, 108, 32, 93, 32, 61, 10, 
    32, 32, 32, 32, 97, 100, 100, 95, 112, 114, 101, 108, 91, 32, 39, 35, 
    105, 102, 100, 101, 102, 32, 83, 73, 80, 69, 95, 77, 65, 73, 78, 39, 
    32, 93, 10, 32, 32, 32, 32, 97, 100, 100, 95, 112, 114, 101, 108, 91, 
    32, 39, 35, 105, 110, 99, 108, 117, 100, 101, 32, 60, 105, 111, 115, 116, 
    114, 101, 97, 109, 62, 39, 32, 93, 10, 32, 32, 32, 32, 97, 100, 100, 
    95, 112, 114, 101, 108, 91, 32, 39, 35, 101, 110, 100, 105, 102, 32, 47, 
    47, 32, 83, 73, 80, 69, 95, 77, 65, 73, 78, 39, 32, 93, 10, 32, 
    32, 32, 32, 123, 32, 115, 116, 100, 58, 58, 99, 111, 117, 116, 32, 60, 
    60, 32, 34, 118, 97, 108, 32, 45, 62, 32, 39, 34, 32, 60, 60, 32, 
    115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 118, 97, 108, 32, 60, 
    60, 32, 34, 39, 34, 32, 60, 60, 32, 115, 116, 100, 58, 58, 101, 110, 
    100, 108, 59, 32, 125, 10, 10, 35, 32, 112, 114, 105, 110, 116, 32, 97, 
    32, 118, 97, 108, 117, 101, 32, 40, 98, 121, 32, 100, 101, 102, 97, 117, 
    108, 116, 44, 32, 112, 114, 105, 110, 116, 32, 99, 117, 114, 114, 101, 110, 
    116, 32, 99, 104, 97, 114, 41, 10, 100, 91, 32, 95, 95, 118, 97, 108, 
    95, 95, 32, 93, 32, 61, 10, 32, 32, 32, 32, 97, 100, 100, 95, 112, 
    114, 101, 108, 91, 32, 39, 35, 105, 102, 100, 101, 102, 32, 83, 73, 80, 
    69, 95, 77, 65, 73, 78, 39, 32, 93, 10, 32, 32, 32, 32, 97, 100, 
    100, 95, 112, 114, 101, 108, 91, 32, 39, 35, 105, 110, 99, 108, 117, 100, 
    101, 32, 60, 105, 111, 115, 116, 114, 101, 97, 109, 62, 39, 32, 93, 10, 
    32, 32, 32, 32, 97, 100, 100, 95, 112, 114, 101, 108, 91, 32, 39, 35, 
    101, 110, 100, 105, 102, 32, 47, 47, 32, 83, 73, 80, 69, 95, 77, 65, 
    73, 78, 39, 32, 93, 10, 32, 32, 32, 32, 123, 32, 115, 116, 100, 58, 
    58, 99, 111, 117, 116, 32, 60, 60, 32, 115, 105, 112, 101, 95, 100, 97, 
    116, 97, 45, 62, 95, 95, 118, 97, 108, 95, 95, 59, 32, 125, 10, 10, 
    35, 32, 112, 114, 105, 110, 116, 32, 97, 32, 118, 97, 108, 117, 101, 32, 
    40, 98, 121, 32, 100, 101, 102, 97, 117, 108, 116, 44, 32, 112, 114, 105, 
    110, 116, 32, 99, 117, 114, 114, 101, 110, 116, 32, 99, 104, 97, 114, 41, 
    10, 99, 32, 61, 10, 32, 32, 32, 32, 97, 100, 100, 95, 112, 114, 101, 
    108, 91, 32, 39, 35, 105, 102, 100, 101, 102, 32, 83, 73, 80, 69, 95, 
    77, 65, 73, 78, 39, 32, 93, 10, 32, 32, 32, 32, 97, 100, 100, 95, 
    112, 114, 101, 108, 91, 32, 39, 35, 105, 110, 99, 108, 117, 100, 101, 32, 
    60, 105, 111, 115, 116, 114, 101, 97, 109, 62, 39, 32, 93, 10, 32, 32, 
    32, 32, 97, 100, 100, 95, 112, 114, 101, 108, 91, 32, 39, 35, 101, 110, 
    100, 105, 102, 32, 47, 47, 32, 83, 73, 80, 69, 95, 77, 65, 73, 78, 
    39, 32, 93, 10, 32, 32, 32, 32, 123, 32, 115, 116, 100, 58, 58, 99, 
    111, 117, 116, 32, 60, 60, 32, 42, 100, 97, 116, 97, 59, 32, 125, 10, 
    10, 35, 32, 115, 104, 111, 117, 108, 100, 32, 98, 101, 32, 102, 111, 108, 
    108, 111, 119, 101, 100, 32, 98, 121, 32, 97, 32, 115, 116, 111, 112, 105, 
    110, 103, 32, 99, 111, 110, 100, 105, 116, 105, 111, 110, 32, 40, 101, 46, 
    103, 46, 32, 39, 32, 39, 41, 10, 119, 111, 114, 100, 91, 32, 95, 95, 
    118, 97, 114, 95, 95, 32, 93, 32, 61, 10, 32, 32, 32, 32, 97, 100, 
    100, 95, 112, 114, 101, 108, 91, 32, 39, 35, 105, 102, 100, 101, 102, 32, 
    83, 73, 80, 69, 95, 77, 65, 73, 78, 39, 32, 93, 10, 32, 32, 32, 
    32, 97, 100, 100, 95, 112, 114, 101, 108, 91, 32, 39, 35, 105, 110, 99, 
    108, 117, 100, 101, 32, 60, 115, 116, 114, 105, 110, 103, 62, 39, 32, 93, 
    10, 32, 32, 32, 32, 97, 100, 100, 95, 112, 114, 101, 108, 91, 32, 39, 
    35, 101, 110, 100, 105, 102, 32, 47, 47, 32, 83, 73, 80, 69, 95, 77, 
    65, 73, 78, 39, 32, 93, 10, 32, 32, 32, 32, 97, 100, 100, 95, 97, 
    116, 116, 114, 91, 32, 39, 115, 116, 100, 58, 58, 115, 116, 114, 105, 110, 
    103, 32, 95, 95, 118, 97, 114, 95, 95, 59, 39, 32, 93, 10, 32, 32, 
    32, 32, 123, 32, 115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 95, 
    95, 118, 97, 114, 95, 95, 46, 99, 108, 101, 97, 114, 40, 41, 59, 32, 
    125, 10, 32, 32, 32, 32, 40, 32, 97, 110, 121, 32, 123, 32, 115, 105, 
    112, 101, 95, 100, 97, 116, 97, 45, 62, 95, 95, 118, 97, 114, 95, 95, 
    32, 43, 61, 32, 42, 100, 97, 116, 97, 59, 32, 125, 32, 41, 42, 10, 
    10, 101, 119, 111, 114, 100, 91, 32, 95, 95, 118, 97, 114, 95, 95, 44, 
    32, 95, 95, 101, 110, 100, 95, 95, 32, 93, 32, 61, 10, 32, 32, 32, 
    32, 97, 100, 100, 95, 97, 116, 116, 114, 91, 32, 39, 115, 116, 100, 58, 
    58, 115, 116, 114, 105, 110, 103, 32, 95, 95, 118, 97, 114, 95, 95, 59, 
    39, 32, 93, 10, 32, 32, 32, 32, 123, 32, 115, 105, 112, 101, 95, 100, 
    97, 116, 97, 45, 62, 95, 95, 118, 97, 114, 95, 95, 46, 99, 108, 101, 
    97, 114, 40, 41, 59, 32, 125, 10, 32, 32, 32, 32, 40, 32, 97, 110, 
    121, 32, 45, 32, 39, 95, 95, 101, 110, 100, 95, 95, 39, 32, 123, 32, 
    115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 95, 95, 118, 97, 114, 
    95, 95, 32, 43, 61, 32, 42, 100, 97, 116, 97, 59, 32, 125, 32, 41, 
    42, 42, 10, 10, 10, 35, 32, 114, 101, 97, 100, 32, 98, 105, 110, 97, 
    114, 121, 32, 105, 110, 116, 32, 108, 105, 116, 116, 108, 101, 32, 101, 110, 
    100, 105, 97, 110, 10, 117, 105, 110, 116, 56, 95, 98, 105, 110, 95, 108, 
    101, 91, 32, 95, 95, 118, 97, 108, 95, 95, 32, 93, 32, 61, 10, 32, 
    32, 32, 32, 97, 100, 100, 95, 97, 116, 116, 114, 91, 32, 39, 117, 110, 
    115, 105, 103, 110, 101, 100, 32, 99, 104, 97, 114, 32, 95, 95, 118, 97, 
    108, 95, 95, 59, 39, 32, 39, 115, 105, 112, 101, 95, 100, 97, 116, 97, 
    45, 62, 95, 95, 118, 97, 108, 95, 95, 32, 61, 32, 48, 59, 39, 32, 
    93, 10, 32, 32, 32, 32, 97, 110, 121, 32, 123, 32, 115, 105, 112, 101, 
    95, 100, 97, 116, 97, 45, 62, 95, 95, 118, 97, 108, 95, 95, 32, 32, 
    61, 32, 42, 114, 101, 105, 110, 116, 101, 114, 112, 114, 101, 116, 95, 99, 
    97, 115, 116, 60, 99, 111, 110, 115, 116, 32, 117, 110, 115, 105, 103, 110, 
    101, 100, 32, 99, 104, 97, 114, 32, 42, 62, 40, 32, 100, 97, 116, 97, 
    32, 41, 32, 60, 60, 32, 32, 48, 59, 32, 125, 10, 10, 35, 32, 114, 
    101, 97, 100, 32, 98, 105, 110, 97, 114, 121, 32, 105, 110, 116, 32, 108, 
    105, 116, 116, 108, 101, 32, 101, 110, 100, 105, 97, 110, 10, 117, 105, 110, 
    116, 49, 54, 95, 98, 105, 110, 95, 108, 101, 91, 32, 95, 95, 118, 97, 
    108, 95, 95, 32, 93, 32, 61, 10, 32, 32, 32, 32, 97, 100, 100, 95, 
    97, 116, 116, 114, 91, 32, 39, 117, 110, 115, 105, 103, 110, 101, 100, 32, 
    115, 104, 111, 114, 116, 32, 95, 95, 118, 97, 108, 95, 95, 59, 39, 32, 
    39, 115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 95, 95, 118, 97, 
    108, 95, 95, 32, 61, 32, 48, 59, 39, 32, 93, 10, 32, 32, 32, 32, 
    97, 110, 121, 32, 123, 32, 115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 
    62, 95, 95, 118, 97, 108, 95, 95, 32, 32, 61, 32, 42, 114, 101, 105, 
    110, 116, 101, 114, 112, 114, 101, 116, 95, 99, 97, 115, 116, 60, 99, 111, 
    110, 115, 116, 32, 117, 110, 115, 105, 103, 110, 101, 100, 32, 99, 104, 97, 
    114, 32, 42, 62, 40, 32, 100, 97, 116, 97, 32, 41, 32, 60, 60, 32, 
    32, 48, 59, 32, 125, 10, 32, 32, 32, 32, 97, 110, 121, 32, 123, 32, 
    115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 95, 95, 118, 97, 108, 
    95, 95, 32, 43, 61, 32, 42, 114, 101, 105, 110, 116, 101, 114, 112, 114, 
    101, 116, 95, 99, 97, 115, 116, 60, 99, 111, 110, 115, 116, 32, 117, 110, 
    115, 105, 103, 110, 101, 100, 32, 99, 104, 97, 114, 32, 42, 62, 40, 32, 
    100, 97, 116, 97, 32, 41, 32, 60, 60, 32, 32, 56, 59, 32, 125, 10, 
    10, 35, 32, 114, 101, 97, 100, 32, 98, 105, 110, 97, 114, 121, 32, 105, 
    110, 116, 32, 108, 105, 116, 116, 108, 101, 32, 101, 110, 100, 105, 97, 110, 
    10, 117, 105, 110, 116, 51, 50, 95, 98, 105, 110, 95, 108, 101, 91, 32, 
    95, 95, 118, 97, 108, 95, 95, 32, 93, 32, 61, 10, 32, 32, 32, 32, 
    97, 100, 100, 95, 97, 116, 116, 114, 91, 32, 39, 117, 110, 115, 105, 103, 
    110, 101, 100, 32, 95, 95, 118, 97, 108, 95, 95, 59, 39, 32, 39, 115, 
    105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 95, 95, 118, 97, 108, 95, 
    95, 32, 61, 32, 48, 59, 39, 32, 93, 10, 32, 32, 32, 32, 97, 110, 
    121, 32, 123, 32, 115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 95, 
    95, 118, 97, 108, 95, 95, 32, 32, 61, 32, 42, 114, 101, 105, 110, 116, 
    101, 114, 112, 114, 101, 116, 95, 99, 97, 115, 116, 60, 99, 111, 110, 115, 
    116, 32, 117, 110, 115, 105, 103, 110, 101, 100, 32, 99, 104, 97, 114, 32, 
    42, 62, 40, 32, 100, 97, 116, 97, 32, 41, 32, 60, 60, 32, 32, 48, 
    59, 32, 125, 10, 32, 32, 32, 32, 97, 110, 121, 32, 123, 32, 115, 105, 
    112, 101, 95, 100, 97, 116, 97, 45, 62, 95, 95, 118, 97, 108, 95, 95, 
    32, 43, 61, 32, 42, 114, 101, 105, 110, 116, 101, 114, 112, 114, 101, 116, 
    95, 99, 97, 115, 116, 60, 99, 111, 110, 115, 116, 32, 117, 110, 115, 105, 
    103, 110, 101, 100, 32, 99, 104, 97, 114, 32, 42, 62, 40, 32, 100, 97, 
    116, 97, 32, 41, 32, 60, 60, 32, 32, 56, 59, 32, 125, 10, 32, 32, 
    32, 32, 97, 110, 121, 32, 123, 32, 115, 105, 112, 101, 95, 100, 97, 116, 
    97, 45, 62, 95, 95, 118, 97, 108, 95, 95, 32, 43, 61, 32, 42, 114, 
    101, 105, 110, 116, 101, 114, 112, 114, 101, 116, 95, 99, 97, 115, 116, 60, 
    99, 111, 110, 115, 116, 32, 117, 110, 115, 105, 103, 110, 101, 100, 32, 99, 
    104, 97, 114, 32, 42, 62, 40, 32, 100, 97, 116, 97, 32, 41, 32, 60, 
    60, 32, 49, 54, 59, 32, 125, 10, 32, 32, 32, 32, 97, 110, 121, 32, 
    123, 32, 115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 95, 95, 118, 
    97, 108, 95, 95, 32, 43, 61, 32, 42, 114, 101, 105, 110, 116, 101, 114, 
    112, 114, 101, 116, 95, 99, 97, 115, 116, 60, 99, 111, 110, 115, 116, 32, 
    117, 110, 115, 105, 103, 110, 101, 100, 32, 99, 104, 97, 114, 32, 42, 62, 
    40, 32, 100, 97, 116, 97, 32, 41, 32, 60, 60, 32, 50, 52, 59, 32, 
    125, 10, 10, 35, 32, 114, 101, 97, 100, 32, 98, 105, 110, 97, 114, 121, 
    32, 105, 110, 116, 32, 108, 105, 116, 116, 108, 101, 32, 101, 110, 100, 105, 
    97, 110, 10, 117, 105, 110, 116, 54, 52, 95, 98, 105, 110, 95, 108, 101, 
    91, 32, 95, 95, 118, 97, 108, 95, 95, 44, 32, 95, 95, 116, 121, 112, 
    101, 95, 95, 32, 61, 32, 39, 113, 117, 105, 110, 116, 54, 52, 39, 32, 
    93, 32, 61, 10, 32, 32, 32, 32, 97, 100, 100, 95, 97, 116, 116, 114, 
    91, 32, 39, 95, 95, 116, 121, 112, 101, 95, 95, 32, 95, 95, 118, 97, 
    108, 95, 95, 59, 39, 32, 39, 115, 105, 112, 101, 95, 100, 97, 116, 97, 
    45, 62, 95, 95, 118, 97, 108, 95, 95, 32, 61, 32, 48, 59, 39, 32, 
    93, 10, 32, 32, 32, 32, 97, 110, 121, 32, 123, 32, 115, 105, 112, 101, 
    95, 100, 97, 116, 97, 45, 62, 95, 95, 118, 97, 108, 95, 95, 32, 32, 
    61, 32, 40, 95, 95, 116, 121, 112, 101, 95, 95, 41, 42, 114, 101, 105, 
    110, 116, 101, 114, 112, 114, 101, 116, 95, 99, 97, 115, 116, 60, 99, 111, 
    110, 115, 116, 32, 117, 110, 115, 105, 103, 110, 101, 100, 32, 99, 104, 97, 
    114, 32, 42, 62, 40, 32, 100, 97, 116, 97, 32, 41, 32, 60, 60, 32, 
    32, 48, 59, 32, 125, 10, 32, 32, 32, 32, 97, 110, 121, 32, 123, 32, 
    115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 95, 95, 118, 97, 108, 
    95, 95, 32, 43, 61, 32, 40, 95, 95, 116, 121, 112, 101, 95, 95, 41, 
    42, 114, 101, 105, 110, 116, 101, 114, 112, 114, 101, 116, 95, 99, 97, 115, 
    116, 60, 99, 111, 110, 115, 116, 32, 117, 110, 115, 105, 103, 110, 101, 100, 
    32, 99, 104, 97, 114, 32, 42, 62, 40, 32, 100, 97, 116, 97, 32, 41, 
    32, 60, 60, 32, 32, 56, 59, 32, 125, 10, 32, 32, 32, 32, 97, 110, 
    121, 32, 123, 32, 115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 95, 
    95, 118, 97, 108, 95, 95, 32, 43, 61, 32, 40, 95, 95, 116, 121, 112, 
    101, 95, 95, 41, 42, 114, 101, 105, 110, 116, 101, 114, 112, 114, 101, 116, 
    95, 99, 97, 115, 116, 60, 99, 111, 110, 115, 116, 32, 117, 110, 115, 105, 
    103, 110, 101, 100, 32, 99, 104, 97, 114, 32, 42, 62, 40, 32, 100, 97, 
    116, 97, 32, 41, 32, 60, 60, 32, 49, 54, 59, 32, 125, 10, 32, 32, 
    32, 32, 97, 110, 121, 32, 123, 32, 115, 105, 112, 101, 95, 100, 97, 116, 
    97, 45, 62, 95, 95, 118, 97, 108, 95, 95, 32, 43, 61, 32, 40, 95, 
    95, 116, 121, 112, 101, 95, 95, 41, 42, 114, 101, 105, 110, 116, 101, 114, 
    112, 114, 101, 116, 95, 99, 97, 115, 116, 60, 99, 111, 110, 115, 116, 32, 
    117, 110, 115, 105, 103, 110, 101, 100, 32, 99, 104, 97, 114, 32, 42, 62, 
    40, 32, 100, 97, 116, 97, 32, 41, 32, 60, 60, 32, 50, 52, 59, 32, 
    125, 10, 32, 32, 32, 32, 97, 110, 121, 32, 123, 32, 115, 105, 112, 101, 
    95, 100, 97, 116, 97, 45, 62, 95, 95, 118, 97, 108, 95, 95, 32, 43, 
    61, 32, 40, 95, 95, 116, 121, 112, 101, 95, 95, 41, 42, 114, 101, 105, 
    110, 116, 101, 114, 112, 114, 101, 116, 95, 99, 97, 115, 116, 60, 99, 111, 
    110, 115, 116, 32, 117, 110, 115, 105, 103, 110, 101, 100, 32, 99, 104, 97, 
    114, 32, 42, 62, 40, 32, 100, 97, 116, 97, 32, 41, 32, 60, 60, 32, 
    51, 50, 59, 32, 125, 10, 32, 32, 32, 32, 97, 110, 121, 32, 123, 32, 
    115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 95, 95, 118, 97, 108, 
    95, 95, 32, 43, 61, 32, 40, 95, 95, 116, 121, 112, 101, 95, 95, 41, 
    42, 114, 101, 105, 110, 116, 101, 114, 112, 114, 101, 116, 95, 99, 97, 115, 
    116, 60, 99, 111, 110, 115, 116, 32, 117, 110, 115, 105, 103, 110, 101, 100, 
    32, 99, 104, 97, 114, 32, 42, 62, 40, 32, 100, 97, 116, 97, 32, 41, 
    32, 60, 60, 32, 52, 48, 59, 32, 125, 10, 32, 32, 32, 32, 97, 110, 
    121, 32, 123, 32, 115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 95, 
    95, 118, 97, 108, 95, 95, 32, 43, 61, 32, 40, 95, 95, 116, 121, 112, 
    101, 95, 95, 41, 42, 114, 101, 105, 110, 116, 101, 114, 112, 114, 101, 116, 
    95, 99, 97, 115, 116, 60, 99, 111, 110, 115, 116, 32, 117, 110, 115, 105, 
    103, 110, 101, 100, 32, 99, 104, 97, 114, 32, 42, 62, 40, 32, 100, 97, 
    116, 97, 32, 41, 32, 60, 60, 32, 52, 56, 59, 32, 125, 10, 32, 32, 
    32, 32, 97, 110, 121, 32, 123, 32, 115, 105, 112, 101, 95, 100, 97, 116, 
    97, 45, 62, 95, 95, 118, 97, 108, 95, 95, 32, 43, 61, 32, 40, 95, 
    95, 116, 121, 112, 101, 95, 95, 41, 42, 114, 101, 105, 110, 116, 101, 114, 
    112, 114, 101, 116, 95, 99, 97, 115, 116, 60, 99, 111, 110, 115, 116, 32, 
    117, 110, 115, 105, 103, 110, 101, 100, 32, 99, 104, 97, 114, 32, 42, 62, 
    40, 32, 100, 97, 116, 97, 32, 41, 32, 60, 60, 32, 53, 54, 59, 32, 
    125, 10, 10, 35, 32, 114, 101, 97, 100, 32, 98, 105, 110, 97, 114, 121, 
    32, 105, 110, 116, 32, 108, 105, 116, 116, 108, 101, 32, 101, 110, 100, 105, 
    97, 110, 10, 105, 110, 116, 54, 52, 95, 98, 105, 110, 95, 108, 101, 91, 
    32, 95, 95, 118, 97, 108, 95, 95, 44, 32, 95, 95, 116, 121, 112, 101, 
    95, 95, 32, 61, 32, 39, 113, 105, 110, 116, 54, 52, 39, 32, 93, 32, 
    61, 10, 32, 32, 32, 32, 97, 100, 100, 95, 97, 116, 116, 114, 91, 32, 
    39, 95, 95, 116, 121, 112, 101, 95, 95, 32, 95, 95, 118, 97, 108, 95, 
    95, 59, 39, 32, 39, 115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 
    95, 95, 118, 97, 108, 95, 95, 32, 61, 32, 48, 59, 39, 32, 93, 10, 
    32, 32, 32, 32, 97, 110, 121, 32, 123, 32, 115, 105, 112, 101, 95, 100, 
    97, 116, 97, 45, 62, 95, 95, 118, 97, 108, 95, 95, 32, 32, 61, 32, 
    40, 95, 95, 116, 121, 112, 101, 95, 95, 41, 42, 114, 101, 105, 110, 116, 
    101, 114, 112, 114, 101, 116, 95, 99, 97, 115, 116, 60, 99, 111, 110, 115, 
    116, 32, 117, 110, 115, 105, 103, 110, 101, 100, 32, 99, 104, 97, 114, 32, 
    42, 62, 40, 32, 100, 97, 116, 97, 32, 41, 32, 60, 60, 32, 32, 48, 
    59, 32, 125, 10, 32, 32, 32, 32, 97, 110, 121, 32, 123, 32, 115, 105, 
    112, 101, 95, 100, 97, 116, 97, 45, 62, 95, 95, 118, 97, 108, 95, 95, 
    32, 43, 61, 32, 40, 95, 95, 116, 121, 112, 101, 95, 95, 41, 42, 114, 
    101, 105, 110, 116, 101, 114, 112, 114, 101, 116, 95, 99, 97, 115, 116, 60, 
    99, 111, 110, 115, 116, 32, 117, 110, 115, 105, 103, 110, 101, 100, 32, 99, 
    104, 97, 114, 32, 42, 62, 40, 32, 100, 97, 116, 97, 32, 41, 32, 60, 
    60, 32, 32, 56, 59, 32, 125, 10, 32, 32, 32, 32, 97, 110, 121, 32, 
    123, 32, 115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 95, 95, 118, 
    97, 108, 95, 95, 32, 43, 61, 32, 40, 95, 95, 116, 121, 112, 101, 95, 
    95, 41, 42, 114, 101, 105, 110, 116, 101, 114, 112, 114, 101, 116, 95, 99, 
    97, 115, 116, 60, 99, 111, 110, 115, 116, 32, 117, 110, 115, 105, 103, 110, 
    101, 100, 32, 99, 104, 97, 114, 32, 42, 62, 40, 32, 100, 97, 116, 97, 
    32, 41, 32, 60, 60, 32, 49, 54, 59, 32, 125, 10, 32, 32, 32, 32, 
    97, 110, 121, 32, 123, 32, 115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 
    62, 95, 95, 118, 97, 108, 95, 95, 32, 43, 61, 32, 40, 95, 95, 116, 
    121, 112, 101, 95, 95, 41, 42, 114, 101, 105, 110, 116, 101, 114, 112, 114, 
    101, 116, 95, 99, 97, 115, 116, 60, 99, 111, 110, 115, 116, 32, 117, 110, 
    115, 105, 103, 110, 101, 100, 32, 99, 104, 97, 114, 32, 42, 62, 40, 32, 
    100, 97, 116, 97, 32, 41, 32, 60, 60, 32, 50, 52, 59, 32, 125, 10, 
    32, 32, 32, 32, 97, 110, 121, 32, 123, 32, 115, 105, 112, 101, 95, 100, 
    97, 116, 97, 45, 62, 95, 95, 118, 97, 108, 95, 95, 32, 43, 61, 32, 
    40, 95, 95, 116, 121, 112, 101, 95, 95, 41, 42, 114, 101, 105, 110, 116, 
    101, 114, 112, 114, 101, 116, 95, 99, 97, 115, 116, 60, 99, 111, 110, 115, 
    116, 32, 117, 110, 115, 105, 103, 110, 101, 100, 32, 99, 104, 97, 114, 32, 
    42, 62, 40, 32, 100, 97, 116, 97, 32, 41, 32, 60, 60, 32, 51, 50, 
    59, 32, 125, 10, 32, 32, 32, 32, 97, 110, 121, 32, 123, 32, 115, 105, 
    112, 101, 95, 100, 97, 116, 97, 45, 62, 95, 95, 118, 97, 108, 95, 95, 
    32, 43, 61, 32, 40, 95, 95, 116, 121, 112, 101, 95, 95, 41, 42, 114, 
    101, 105, 110, 116, 101, 114, 112, 114, 101, 116, 95, 99, 97, 115, 116, 60, 
    99, 111, 110, 115, 116, 32, 117, 110, 115, 105, 103, 110, 101, 100, 32, 99, 
    104, 97, 114, 32, 42, 62, 40, 32, 100, 97, 116, 97, 32, 41, 32, 60, 
    60, 32, 52, 48, 59, 32, 125, 10, 32, 32, 32, 32, 97, 110, 121, 32, 
    123, 32, 115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 95, 95, 118, 
    97, 108, 95, 95, 32, 43, 61, 32, 40, 95, 95, 116, 121, 112, 101, 95, 
    95, 41, 42, 114, 101, 105, 110, 116, 101, 114, 112, 114, 101, 116, 95, 99, 
    97, 115, 116, 60, 99, 111, 110, 115, 116, 32, 117, 110, 115, 105, 103, 110, 
    101, 100, 32, 99, 104, 97, 114, 32, 42, 62, 40, 32, 100, 97, 116, 97, 
    32, 41, 32, 60, 60, 32, 52, 56, 59, 32, 125, 10, 32, 32, 32, 32, 
    97, 110, 121, 32, 123, 32, 115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 
    62, 95, 95, 118, 97, 108, 95, 95, 32, 43, 61, 32, 40, 95, 95, 116, 
    121, 112, 101, 95, 95, 41, 42, 114, 101, 105, 110, 116, 101, 114, 112, 114, 
    101, 116, 95, 99, 97, 115, 116, 60, 99, 111, 110, 115, 116, 32, 117, 110, 
    115, 105, 103, 110, 101, 100, 32, 99, 104, 97, 114, 32, 42, 62, 40, 32, 
    100, 97, 116, 97, 32, 41, 32, 60, 60, 32, 53, 54, 59, 32, 125, 10, 
    10, 35, 32, 114, 101, 97, 100, 32, 97, 32, 115, 116, 114, 105, 110, 103, 
    32, 40, 99, 104, 97, 114, 32, 42, 41, 10, 115, 116, 114, 95, 115, 105, 
    122, 101, 100, 91, 32, 95, 95, 118, 97, 108, 95, 95, 44, 32, 95, 95, 
    108, 101, 110, 95, 95, 44, 32, 95, 95, 116, 121, 112, 101, 95, 95, 32, 
    61, 32, 34, 115, 116, 100, 58, 58, 115, 116, 114, 105, 110, 103, 34, 32, 
    93, 32, 61, 10, 32, 32, 32, 32, 97, 100, 100, 95, 97, 116, 116, 114, 
    91, 32, 39, 95, 95, 116, 121, 112, 101, 95, 95, 32, 95, 95, 118, 97, 
    108, 95, 95, 59, 39, 32, 93, 10, 32, 32, 32, 32, 97, 100, 100, 95, 
    97, 116, 116, 114, 91, 32, 39, 105, 110, 116, 32, 95, 95, 118, 97, 108, 
    95, 95, 95, 114, 101, 109, 59, 39, 32, 93, 10, 32, 32, 32, 32, 123, 
    10, 32, 32, 32, 32, 32, 32, 32, 32, 115, 105, 112, 101, 95, 100, 97, 
    116, 97, 45, 62, 95, 95, 118, 97, 108, 95, 95, 95, 114, 101, 109, 32, 
    61, 32, 95, 95, 108, 101, 110, 95, 95, 59, 10, 32, 32, 32, 32, 32, 
    32, 32, 32, 115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 95, 95, 
    118, 97, 108, 95, 95, 46, 99, 108, 101, 97, 114, 40, 41, 59, 10, 32, 
    32, 32, 32, 32, 32, 32, 32, 115, 105, 112, 101, 95, 100, 97, 116, 97, 
    45, 62, 95, 95, 118, 97, 108, 95, 95, 46, 114, 101, 115, 101, 114, 118, 
    101, 40, 32, 95, 95, 108, 101, 110, 95, 95, 32, 41, 59, 10, 32, 32, 
    32, 32, 106, 109, 112, 95, 114, 100, 95, 95, 95, 118, 97, 108, 95, 95, 
    36, 36, 36, 117, 105, 100, 36, 36, 36, 58, 10, 32, 32, 32, 32, 32, 
    32, 32, 32, 105, 102, 32, 40, 32, 110, 111, 116, 32, 115, 105, 112, 101, 
    95, 100, 97, 116, 97, 45, 62, 95, 95, 118, 97, 108, 95, 95, 95, 114, 
    101, 109, 32, 41, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 
    32, 103, 111, 116, 111, 32, 101, 110, 100, 95, 114, 100, 95, 95, 95, 118, 
    97, 108, 95, 95, 36, 36, 36, 117, 105, 100, 36, 36, 36, 59, 10, 32, 
    32, 32, 32, 32, 32, 32, 32, 105, 102, 32, 40, 32, 43, 43, 100, 97, 
    116, 97, 32, 62, 61, 32, 101, 110, 100, 32, 41, 32, 123, 10, 32, 32, 
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 115, 105, 112, 101, 95, 100, 
    97, 116, 97, 45, 62, 95, 105, 110, 112, 95, 99, 111, 110, 116, 32, 61, 
    32, 38, 38, 99, 110, 116, 95, 114, 100, 95, 95, 95, 118, 97, 108, 95, 
    95, 36, 36, 36, 117, 105, 100, 36, 36, 36, 59, 10, 32, 32, 32, 32, 
    32, 32, 32, 32, 32, 32, 32, 32, 114, 101, 116, 117, 114, 110, 32, 48, 
    59, 10, 32, 32, 32, 32, 32, 32, 32, 32, 125, 10, 32, 32, 32, 32, 
    99, 110, 116, 95, 114, 100, 95, 95, 95, 118, 97, 108, 95, 95, 36, 36, 
    36, 117, 105, 100, 36, 36, 36, 58, 10, 32, 32, 32, 32, 32, 32, 32, 
    32, 32, 32, 32, 32, 115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 
    95, 95, 118, 97, 108, 95, 95, 32, 43, 61, 32, 42, 100, 97, 116, 97, 
    59, 10, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 45, 45, 
    115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 62, 95, 95, 118, 97, 108, 
    95, 95, 95, 114, 101, 109, 59, 10, 32, 32, 32, 32, 32, 32, 32, 32, 
    32, 32, 32, 32, 103, 111, 116, 111, 32, 106, 109, 112, 95, 114, 100, 95, 
    95, 95, 118, 97, 108, 95, 95, 36, 36, 36, 117, 105, 100, 36, 36, 36, 
    59, 10, 32, 32, 32, 32, 101, 110, 100, 95, 114, 100, 95, 95, 95, 118, 
    97, 108, 95, 95, 36, 36, 36, 117, 105, 100, 36, 36, 36, 58, 32, 59, 
    10, 32, 32, 32, 32, 125, 10, 10, 35, 32, 115, 105, 122, 101, 32, 51, 
    50, 32, 98, 105, 116, 115, 32, 116, 104, 101, 110, 32, 100, 97, 116, 97, 
    10, 115, 116, 114, 95, 51, 50, 91, 32, 95, 95, 108, 101, 110, 95, 95, 
    32, 61, 32, 39, 108, 101, 110, 39, 44, 32, 95, 95, 115, 116, 114, 95, 
    95, 32, 61, 32, 39, 115, 116, 114, 39, 32, 93, 32, 61, 10, 32, 32, 
    32, 32, 117, 105, 110, 116, 51, 50, 95, 98, 105, 110, 95, 108, 101, 91, 
    32, 39, 95, 95, 108, 101, 110, 95, 95, 39, 32, 93, 10, 32, 32, 32, 
    32, 115, 116, 114, 95, 115, 105, 122, 101, 100, 91, 32, 95, 95, 115, 116, 
    114, 95, 95, 44, 32, 39, 115, 105, 112, 101, 95, 100, 97, 116, 97, 45, 
    62, 95, 95, 108, 101, 110, 95, 95, 39, 32, 93, 10, 0
};

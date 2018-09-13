
//cypher query language
//eur-lex
//Pedro V


///////////////Defining the Graph///////////////

///https://s3.eu-central-1.amazonaws.com/maastrichtuniversity-ids-open/lex2rdf/cases_full.csv
///https://s3.eu-central-1.amazonaws.com/maastrichtuniversity-ids-open/lex2rdf/cases_test.csv
//Added 3392 labels, created 3392 nodes, set 20837 properties, created 14329 relationships, completed after 622188 ms.

source,target,paragraph,subject,country,case_label,ecli,case_type,judge,advocate,country-chamber,chamber,main_subject,lodge_date,document_date,year_document,month_document,year_lodge,month_lodge,case_time,n_countries,joined_cases,ruling_title,ruling_name,ruling_type,ruling_content


//Loading table 
LOAD CSV WITH HEADERS FROM "file:///home/pedrohserrano/cases_test.csv" AS row
//Defining vertex
MERGE (src:Case {
Celex: row.source,
Year: toInt(row.year_document),
Ecli: row.ecli})
MERGE (tgt:Case {
Celex: row.target})

MERGE (j:Judge {
Name: row.judge})

//MERGE (y:Year {Name: toInt(row.year)})

//Defining edges 

(tgt) <- [] - (src)
MERGE (src)-[:cites {fragment: row.paragraph}]->(tgt)


MERGE (src)<-[:Delivered_by]-(j)

ON CREATE SET src.weight = toInt(row.main_subject)
//MERGE (r)-[:Crime {Orig: toInt(row.Crim_o), Dest: toInt(row.Crim_d)}]-(y)




//Loading table 
LOAD CSV WITH HEADERS FROM "file:///home/pedrohserrano/cases_test.csv" AS row
MERGE (src:Case {Celex: row.source, Year: toInt(row.year_document), Ecli: row.ecli})
MERGE (tgt:Case {Celex: row.target})
MERGE (j:Judge {Name: row.judge}) 
MERGE (src)-[:cites {fragment: row.paragraph}]->(tgt)
MERGE (src)-[:Delivered_by]->(j)
ON CREATE SET src.weight = toInt(row.main_subject)






///////////////Graph decriptive analysis///////////////


//Count vertex total
MATCH (n)  
RETURN count(n)

//Count edges total
MATCH ()-[r]->()
RETURN count(r)

//Outdegrees vertex type: Zone
MATCH (n:Zone)-[r:Move_to]->()
RETURN n.Name as Node, count(r) as Outdegree
ORDER BY Outdegree DESC
UNION
MATCH (a:Zone)-[r:Move_to]->(leaf)
WHERE not((leaf)-->())
RETURN leaf.Name as Node, 0 as Outdegree
//On the document it shows rank top 5 and last 5

//Indegrees vertex type: Zone
MATCH (n:Zone)<-[r:Move_to]-()
RETURN n.Name as Node, count(r) as Indegree
ORDER BY Indegree DESC
UNION
MATCH (a:Zone)<-[r:Move_to]-(root)
WHERE not((root)<--())
RETURN root.Name as Node, 0 as Indegree
//On the document it shows rank top 5 and last 5

//Total degrees vertex type: Zone
MATCH (n:Zone)-[r:Move_to]-()
RETURN n.Name, count(distinct r) as Degree
ORDER BY Degree DESC

//Graphs degree Histogram
MATCH (n:Zone)-[r:Move_to]-()
WITH n as Nodes, count(distinct r) as Degree
RETURN Degree, Count(Nodes)
ORDER BY Degree ASC

//Calculate the diameter of the subgraph in which only considers colonies
MATCH (z1:Zone),(z2:Zone)
WHERE z1 <> z2
WITH z1, z2
MATCH p=shortestPath((z1)-[r:Move_to*]->(z2))
RETURN z1.Name, z2.Name, length(p)
ORDER BY length(p) DESC LIMIT 1

//Since we have the the diameter then all shortest paths
MATCH p = allShortestPaths((n:Zone)-[r:Move_to*]-(m:Zone))
WHERE n.Name='Archipelbuurt' AND m.Name = 'Transvaalkwartier-Midden'
RETURN EXTRACT(n IN NODES(p)| n.Name) AS Paths


///////////////Análisis exploratorio del grafo///////////////


//Show up all relations
MATCH (o)-[:Belong]->(r)-[c:Crime]->(y) RETURN o,r,y
//Image 1 in the document

//Show up all edges which have move_to relation ordered
MATCH (n:Zone)-[:Move_to*]->(m:Zone)
RETURN n, m
ORDER BY n.Crime DESC, n.Offenders DESC
LIMIT 10
//En el documento se muestran los 5 distintos

//Show up top 10 people movement
MATCH (n:Zone)-[r:Move_to]->(m:Zone)
WHERE n<>m
RETURN n.Name, r
ORDER BY r.dist DESC, n.Crime DESC, n.Offenders DESC
LIMIT 10
//En el documento va la gráfica y el grafo, ya

//Show up the relation zone belong to region, then to see communities 
MATCH (n:Zone)-[r:Belong]->(m:Reg)
WHERE n<>m
RETURN n, m, r
ORDER BY n.Crime DESC
LIMIT 100
//Is the second graph on the document

//Dijkstras algorithm to the most influential zone
MATCH (n:Zone {Name:'Kerketuinen/Zichtenburg'}), (m:Zone),
path = shortestPath((n:Zone)-[:Move_to*]->(m:Zone))
WITH REDUCE(dist = 0, rel in rels(path) | dist + toInt(rel.dist)) AS distance, path, n, m
RETURN n, m, path, distance
ORDER BY distance desc

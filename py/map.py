import streamlit as st
import geopandas as gpd
import folium
import pandas as pd
import numpy as np

st.title("Geospatial Data Visualization")
st.markdown("Yo momma")

@st.cache_data
def load_broome_geojson():
    url= "https://clst.org"
    census = "https://census.gov"

    ny_tracts = gpd.read_file(census)
    broome_tracts = ny_tracts[ny_tracts['COUNTYFP'] == '007'].copy()
    broome_tracts = broome_tracts.to_crs(epsg=4326)
    return broome_tracts

try:
    with st.spinner("Downloading"):
        broome_tracts = load_broome_geojson()
        st.success("Download complete!")
except Exception as e:
    pass

# Generate or load attributes
@st.cache_data
def get_mock_attributes(tract_ids):
    np.random.seed(42)
    data = {
        'GEOID': tract_ids,
        'Median Income ($)': np.random.randint(35000, 95000, size=len(tract_ids)),
        'Poverty Rate (%)': np.random.uniform(5.0, 35.0, size=len(tract_ids)).round(1),
        'Population Density (sq mi)': np.random.randint(100, 4000, size=len(tract_ids))
    }
    return pd.DataFrame(data)

df_attribute = get_mock_attributes(broome_tracts['GEOID'].to_list())
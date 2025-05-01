import React from 'react';
import { Container, Grid, Card, CardContent, CardActions, Typography, Button, Box, useTheme } from '@mui/material';
import DownloadIcon from '@mui/icons-material/Download';

function DownloadSection({ apps }) {
  const theme = useTheme();
  const isDarkMode = theme.palette.mode === 'dark';

  return (
    <Box id="download" sx={{ py: 8, bgcolor: isDarkMode ? 'background.paper' : '#FCFCFD' }}>
      <Container maxWidth="lg">
        <Typography 
          variant="h3" 
          component="h2" 
          fontWeight="700" 
          sx={{ mb: 1 }}
        >
          Download <span style={{ color: theme.palette.primary.main }}>Hermes Applications</span> 
        </Typography>
        <br />
        <br />

        <Grid container spacing={4}>
          {apps.map((app) => (
            <Grid item xs={12} sm={6} md={4} key={app.name}>
              <Card 
                sx={{ 
                  height: '100%', 
                  display: 'flex', 
                  flexDirection: 'column',
                  borderRadius: 4,
                  backgroundColor: isDarkMode ? 'rgba(30, 32, 37, 0.7)' : 'white',
                  border: `1px solid ${isDarkMode ? 'rgba(255, 255, 255, 0.1)' : 'rgba(224, 234, 255, 0.8)'}`,
                  boxShadow: isDarkMode ? 'none' : '0px 2px 6px rgba(104, 112, 118, 0.04)',
                  position: 'relative',
                  overflow: 'hidden',
                  transition: 'all 0.3s ease',
                  ':hover': { 
                    boxShadow: 6,
                    transform: 'translateY(-4px)'
                  },
                  '&::before': {
                    content: '""',
                    position: 'absolute',
                    bottom: 0,
                    left: 0,
                    right: 0,
                    height: '70%',
                    background: isDarkMode 
                      ? 'linear-gradient(to top, rgba(0, 60, 115, 0.6) 10%, rgba(0, 60, 115, 0.3) 40%, rgba(30, 32, 37, 0) 100%)' 
                      : 'linear-gradient(to top, rgba(147, 205, 252, 0.85) 0%, rgba(255, 255, 255, 0) 100%)',
                    zIndex: 0,
                  },
                }}
              >
                <CardContent sx={{ 
                  flexGrow: 1, 
                  textAlign: 'center',
                  position: 'relative',
                  zIndex: 1 
                }}>
                  <Typography variant="h5" fontWeight={700} gutterBottom sx={{ color: 'hsl(210, 100%, 60%)' }}>
                    {app.name}
                  </Typography>
                  <br />
                  <Box 
                    component="img" 
                    src={app.icon} 
                    alt={app.name} 
                    sx={{ 
                      width: 60, 
                      mb: 2 
                    }} 
                  />
                </CardContent>
                <CardActions sx={{ position: 'relative', zIndex: 1 }}>
                  <Button
                    fullWidth
                    variant="contained"
                    startIcon={<DownloadIcon />}
                    href={app.file}
                    download
                    sx={{ 
                      textTransform: 'none', 
                      borderRadius: 2,
                      py: 1,
                      '&:hover': { backgroundColor: 'primary.dark' } 
                    }}
                  >
                    Download .deb
                  </Button>
                </CardActions>
              </Card>
            </Grid>
          ))}
        </Grid>
      </Container>
    </Box>
  );
}

export default DownloadSection;
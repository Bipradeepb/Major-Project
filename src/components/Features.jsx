import React from 'react';
import { Box, Typography, Grid, Paper, useTheme } from '@mui/material';
import FileUploadIcon from '@mui/icons-material/FileUpload';
import BackupIcon from '@mui/icons-material/Backup';
import SystemUpdateAltIcon from '@mui/icons-material/SystemUpdateAlt';
import DesktopWindowsIcon from '@mui/icons-material/DesktopWindows';

const Feature = ({ icon, title, description }) => {
  const theme = useTheme();
  const isDarkMode = theme.palette.mode === 'dark';
  
  return (
    <Paper 
      variant="outlined"
      sx={{
        p: 3,
        height: '100%',
        borderRadius: 4,
        backgroundColor: isDarkMode ? 'rgba(30, 32, 37, 0.7)' : 'white',
        border: `1px solid ${isDarkMode ? 'rgba(255, 255, 255, 0.1)' : 'rgba(224, 234, 255, 0.8)'}`,
        boxShadow: isDarkMode ? 'none' : '0px 2px 6px rgba(104, 112, 118, 0.04)',
        position: 'relative',
        overflow: 'hidden',
        '&::before': {
          content: '""',
          position: 'absolute',
          bottom: 0,
          left: 0,
          right: 0,
          height: '70%',
          background: isDarkMode 
            ? 'linear-gradient(to top, rgba(0, 60, 115, 0.3) 0%, rgba(30, 32, 37, 0) 100%)' 
            : 'linear-gradient(to top, rgba(236, 246, 255, 0.8) 0%, rgba(255, 255, 255, 0) 100%)',
          zIndex: 0,
        },
      }}
    >
      <Box sx={{ position: 'relative', zIndex: 1 }}>
        <Box 
          sx={{
            mb: 2,
            width: 48,
            height: 48,
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            borderRadius: 3,
            backgroundColor: isDarkMode ? 'rgba(0, 127, 255, 0.2)' : 'rgba(228, 241, 255, 0.8)',
            border: `1px solid ${isDarkMode ? 'rgba(0, 127, 255, 0.3)' : 'rgba(200, 230, 255, 0.6)'}`,
          }}
        >
          {icon}
        </Box>
        <Typography 
          variant="h6" 
          component="h3" 
          fontWeight="600" 
          sx={{ mb: 1 }}
        >
          {title}
        </Typography>
        <Typography 
          variant="body2" 
          color="text.secondary"
        >
          {description}
        </Typography>
      </Box>
    </Paper>
  );
};

const Features = () => {
  const theme = useTheme();
  const isDarkMode = theme.palette.mode === 'dark';
  
  const features = [
    {
      icon: <FileUploadIcon sx={{ color: '#2196f3' }} />,
      title: 'Efficient and reliable file transfer',
      description: "Transfer files quickly across networks with our optimized protocols. Never worry about data integrity or transfer interruptions."
    },
    {
      icon: <BackupIcon sx={{ color: '#2196f3' }} />,
      title: 'Highly available automated backup server support',
      description: 'Set up automatic backups that run seamlessly in the background. Our redundant server architecture ensures your data is always accessible when you need it.'
    },
    {
      icon: <SystemUpdateAltIcon sx={{ color: '#2196f3' }} />,
      title: 'Flexible, platform independent deb installers',
      description: 'Deploy across any Linux distribution with our universal deb packages. Simple installation process that works consistently across different environments.'
    },
    {
      icon: <DesktopWindowsIcon sx={{ color: '#2196f3' }} />,
      title: 'Modern GUI support for users',
      description: 'Intuitive graphical interface that makes complex operations accessible to all users. Clean design with responsive controls that work across all screen sizes.'
    }
  ];

  return (
    <Box id="features" sx={{ py: 8, px: 2, bgcolor: isDarkMode ? 'inherit' : '#FCFCFD' }}>
      <Box sx={{ maxWidth: 1200, mx: 'auto' }}>
        <Typography 
          variant="subtitle1" 
          color="primary" 
          fontWeight="500" 
          sx={{ mb: 1 }}
        >
          Why build with HERMES?
        </Typography>
        
        <Typography 
          variant="h3" 
          component="h2" 
          fontWeight="700" 
          sx={{ mb: 1 }}
        >
          Enjoy <span style={{ color: theme.palette.primary.main }}>fast, resilient File Transfer</span>
        </Typography>
        
        <Typography 
          variant="h3" 
          component="h2" 
          fontWeight="700" 
          sx={{ mb: 6 }}
        >
          
        </Typography>
        
        <Grid container spacing={3}>
          {features.map((feature, index) => (
            <Grid item xs={12} sm={6} md={3} key={index}>
              <Feature 
                icon={feature.icon} 
                title={feature.title} 
                description={feature.description} 
              />
            </Grid>
          ))}
        </Grid>
      </Box>
    </Box>
  );
};

export default Features;